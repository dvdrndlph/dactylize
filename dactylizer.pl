#!/usr/bin/env perl -w
# File: dactylizer.pl
# Author: David Rndolph
# Date: 23 June 2016
# Purpose: Process raw input to produce a MIDI file and corresponding
# abcDF file.
#
use strict;
use warnings;
use Data::Dumper;
use File::Path;
use MIDI;

# The first (lowest) note on full keyboard is A0, which is MIDI 21.
# my $MIDI_OFFSET = 21; # A0
# The microKEY default setting starts at C3.
my $MIDI_OFFSET = 48; # C3

my $RAW_DIR = './output/raw/';
my $MIDI_SUFFIX = '_midi';
my $FINGER_SUFFIX = '_finger';
my $COOKED_DIR = './output/cooked/';

my $RIGHT_HAND = 0;
my $LEFT_HAND = 1;

my %HAND_OUTPUT = (
    $RIGHT_HAND => '>',
    $LEFT_HAND => '<'
);

my $THUMB = 1;
my $INDEX = 2;
my $MIDDLE = 3;
my $RING = 4;
my $LITTLE = 5;
my $PINKIE = 5;

my %CODE_FINGER = (
    16 => $THUMB,
    8 => $INDEX,
    4 => $MIDDLE,
    2 => $RING,
    1 => $LITTLE
);

my %BIT_FINGER = (
    7 => $LITTLE,
    6 => $RING,
    5 => $MIDDLE,
    4 => $INDEX,
    3 => $THUMB
);

sub get_finger_array {
    my ($binary) = @_;
    my @bit = split //, $binary;
    my @finger = ();
    my $bit_num = 0;
    foreach my $bit (@bit) {
        if ($bit[$bit_num]) {
            push @finger, $BIT_FINGER{$bit_num};
        }
        $bit_num++; 
    }
    return @finger;
}

my $File_Time = $ARGV[0];
my $Midi_Path = "${RAW_DIR}${File_Time}${MIDI_SUFFIX}";
my $Finger_Path = "${RAW_DIR}${File_Time}${FINGER_SUFFIX}";
print "Dactylizing your performance. . . .\n";
print "MIDI event file: $Midi_Path\n";
print "Fingering file: $Finger_Path\n";

my $Output_Dir = "./output/cooked";
mkpath $Output_Dir if not -d $Output_Dir;

my @Midi_Event;
open MIDI, "< $Midi_Path" or die "Bad open of $Midi_Path";
# note_on channel=0 note=65 velocity=6 time=904.565069914
while (my $midi_line = <MIDI>) {
    my %midi_event = ();
    my @token = split /\s+/, $midi_line;
    if ($token[0] =~ /^(note_\w+)/) {
        $midi_event{event} = $1;
    } else {
        next;
    }
    
    foreach my $token (@token) {
        next if not $token =~ /=/;
        my ($var, $val) = split '=', $token;
        $midi_event{$var} = $val;
    } 
    push @Midi_Event, \%midi_event;
}
# print Dumper \@Midi_Event;

my %Finger_Event_For_Note;
open FINGERS, "< $Finger_Path" or die "Bad open of $Finger_Path";
# 240: Hand:1 Finger:16 Key:5 State:1 When:904.505608082
while (my $finger_line = <FINGERS>) {
    my @token = split /\s+/, $finger_line;
    shift @token; # Ignore event number
    my %finger_event = ();
    foreach my $token (@token) {
        my ($var, $val) = split ':', $token; 
        $finger_event{$var} = $val;
    }
    my $decimal = $finger_event{Finger};
    my $binary = sprintf('%08b', $decimal);
    $finger_event{Finger} = $binary;
    my @finger_array = get_finger_array($binary);
    $finger_event{Finger_Array} = \@finger_array;
    my $note_number = $finger_event{Key} + $MIDI_OFFSET;
    push @{$Finger_Event_For_Note{$note_number}}, \%finger_event;
}
# print Dumper \%Finger_Event_For_Note;

sub get_note_ending_time {
    my ($onset_event) = @_;
    my $onset_time = $onset_event->{time};
    my $onset_note = $onset_event->{note};
    foreach my $midi_event (@Midi_Event) {
        next if $midi_event->{time} <= $onset_time;
        next if $midi_event->{note} ne $onset_note;
        return $midi_event->{time};
    }
    die "Note never ended."
}

sub get_fingering {
    my ($midi_event) = @_;
    my $onset_time = $midi_event->{time};
    my $note = $midi_event->{note};
    my $ending_time = get_note_ending_time($midi_event);
# print "Curious about $note starting at $onset_time, ending at $ending_time.\n";
# print Dumper $Finger_Event_For_Note{$note};
    my $striking_event;
    my $releasing_event;
   
    while (my $finger_event = shift @{$Finger_Event_For_Note{$note}}) {
# print "Checking fingering event at time $finger_event->{When}\n";
        if ($finger_event->{When} > $ending_time) {
            unshift @{$Finger_Event_For_Note{$note}}, $finger_event;
            last;
        }

        my $finger_count = scalar @{$finger_event->{Finger_Array}};
        if ($finger_event->{State}) {
            if ($finger_count == 1) {
# print "One finger.\n";
                if (not defined $striking_event) {
# print "No striking.\n";
                    $striking_event = $finger_event;
                }
                if ($finger_event->{When} <= $midi_event->{time}) {
                    $striking_event = $finger_event;
                } else {
                    $releasing_event = $finger_event;
                }
            }
        } else {
            # It is a release event. This means no fingers are in
            # contact with the key anymore.
            last;
        }
    }

    my $striking_hand = "";
    if (not $striking_event) {
        return "x";
    }

    my $striking_hand = $HAND_OUTPUT{$striking_event->{Hand}};
    my $striking_finger = $striking_event->{Finger_Array}->[0];
    my $fingering = "${striking_hand}${striking_finger}";
    if (not $releasing_event) {
        return $fingering;
    }

    my $releasing_hand = $HAND_OUTPUT{$releasing_event->{Hand}};
    my $releasing_finger = $releasing_event->{Finger_Array}->[0];
    if ($releasing_hand ne $striking_hand) {
        $fingering .= "-${releasing_hand}${releasing_finger}";
    } elsif ($releasing_finger ne $striking_finger) {
        $fingering .= "-${releasing_finger}";
    }
 
    return $fingering;
}

# Loop through MIDI notes and assign the ``striking" finger as the last
# one to contact the note prior to its onset time. The ``releasing" finger
# is the last finger seen to touch the key prior to its note's ending 
# time stamp, or, if no such finger contact is detected, the striking 
# and releasing fingers are assumed to be the same.
my @Fingering = ();
foreach my $midi_event (@Midi_Event) {
    next if not $midi_event->{event} eq 'note_on';
    my $fingering = get_fingering($midi_event);
    print $fingering;
}

print "\n";

print "Done Dactylizing.\n\n";
