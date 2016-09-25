#!/usr/bin/env perl -w
# Copyright (c) 2016 David A. Randolph.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# File: dactylizer.pl
# Author: David Randolph
# Date: 23 June 2016
# Purpose: Process raw input to produce a MIDI file and corresponding
# abcDF file.
#
use strict;
use English;
use warnings;
use Data::Dumper;
use File::Path;
use List::MoreUtils qw(uniq);
use MIDI;

use constant TRUE => 1;
use constant FALSE => 1;

my $FORMAT_0_CMD = './SMFformat0';
# The first (lowest) note on full keyboard is A0, which is MIDI 21.
my $MIDI_OFFSET = 21; # A0
# The microKEY default setting starts at C3.
# my $MIDI_OFFSET = 48; # C3

my $RAW_DIR = './output/raw/';
my $MIDI_SUFFIX = '_midi';
my $FINGER_SUFFIX = '_finger';
my $COOKED_DIR = './output/cooked/';

my $RIGHT_HAND = 1;
my $LEFT_HAND = 0;

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

mkpath $COOKED_DIR if not -d $COOKED_DIR;
my $Midi_Output_Path = "${COOKED_DIR}${File_Time}.midi";
my $Finger_Output_Path = "${COOKED_DIR}${File_Time}.abcdf";
my $Event_Output_Path = "${COOKED_DIR}${File_Time}.txt";

my @Finger_Event;
my @Midi_Event;
open MIDI, "< $Midi_Path" or die "Bad open of $Midi_Path";
# note_on channel=0 note=65 velocity=6 time=904.565069914
my %Track_For_Note = ();
my %Track_Is_Occupied = ();
my @Track_Events = ();
my $First_Note_Time;

sub get_output_events_for_track {
    my ($track_number) = @_;
    my @event = (['text_event', 0, "${File_Time}_track_${track_number}"]);

    my $prior_raw;
    foreach my $raw_event (@{$Track_Events[$track_number]}) {
        my $delta_time = $raw_event->{time} - $First_Note_Time;
        if ($raw_event->{event} eq 'note_off') {
            die "Missing note_on event" if not defined $prior_raw;
        }
        if (defined $prior_raw) {
            $delta_time = $raw_event->{time} - $prior_raw->{time};
        }
        $delta_time = int($delta_time * 1000 + 0.5);
        push @event, [$raw_event->{event}, $delta_time, 0,
            $raw_event->{note}, $raw_event->{velocity}];
        $prior_raw = $raw_event;
    }
    return \@event;
}

sub assign_to_track {
    my ($event) = @_;
    if ($event->{event} eq 'note_off') {
        my $track_number = $Track_For_Note{$event->{note}};
        push @{$Track_Events[$track_number]}, $event;
        delete $Track_For_Note{$event->{note}};
        $Track_Is_Occupied{$track_number} = 0;
    } elsif ($event->{event} eq 'note_on') {
        my $open_track_index;
        for (my $i = 0; $i < scalar @Track_Events; $i++) {
            next if $Track_Is_Occupied{$i};
            $open_track_index = $i;
        }
        if (not defined $open_track_index) {
            # No room at any inn. Create new track.
            $open_track_index = scalar @Track_Events;
        }
        push @{$Track_Events[$open_track_index]}, $event;
        $Track_For_Note{$event->{note}} = $open_track_index;
        $Track_Is_Occupied{$open_track_index} = 1;
    } # FIXME: Handle pedal events here.
}

sub hash_events {
    my ($events, $key_field) = @_;
    my %event_at_stamp;
    foreach my $event (@{$events}) {
        my $stamp = $event->{$key_field};
        $event_at_stamp{$stamp} = []
            if not defined $event_at_stamp{$stamp};
        push @{$event_at_stamp{$stamp}}, $event;
    }
    return \%event_at_stamp;
}

sub print_interleaved_events {
    my $midi_events = hash_events(\@Midi_Event, 'time');
    my $finger_events = hash_events(\@Finger_Event, 'When');
    my @key = keys(%{$midi_events});
    push @key, keys(%{$finger_events});
    @key = uniq @key;
    foreach my $key (sort {$a <=> $b} @key) {
        my $when = sprintf '%10.5f', $key;
        my $output = '';
        if (defined $midi_events->{$key}) {
            foreach my $event (@{$midi_events->{$key}}) {
                $output .= "At $when MIDI: $event->{note} $event->{event}\n";
            }
        }
        if (defined $finger_events->{$key}) {
            foreach my $event (@{$finger_events->{$key}}) {
                my $note_num = $event->{Key} + $MIDI_OFFSET;
                $output .= "At $when FING: $note_num ";
                if (@{$event->{Finger_Array}}) {
                    $output .= "touched by " . join(',', @{$event->{Finger_Array}});
                } else {
                    $output .= "released";
                }
                $output .= "\n";
            }
        }
        print $output;
        # print Dumper $midi_events->{$key}
        # print Dumper $finger_events->{$key} if defined $finger_events->{$key}; 
    }
}

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
   
    if (not defined $First_Note_Time and $midi_event{event} eq 'note_on') {
        $First_Note_Time = $midi_event{time}; 
    }
    assign_to_track(\%midi_event);
}
close MIDI;

# print Dumper \@Track_Events;

my @Output_Track;
for (my $i = 0; $i < scalar @Track_Events; $i++) {
    my $events = get_output_events_for_track($i);
    my $track = MIDI::Track->new({'events' => $events}); 
    push @Output_Track, $track;
}

my $opus = MIDI::Opus->new({'format' => 1,
    'ticks' => 500,
    'tracks' => \@Output_Track});
$opus->write_to_file($Midi_Output_Path);
system("$FORMAT_0_CMD $Midi_Output_Path $Midi_Output_Path");
die "Bad MIDI format 0 transformation" if $CHILD_ERROR;

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
    push @Finger_Event, \%finger_event;
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
    my $striking_event;
    my $releasing_event;
   
    while (my $finger_event = shift @{$Finger_Event_For_Note{$note}}) {
        if ($finger_event->{When} > $ending_time) {
            unshift @{$Finger_Event_For_Note{$note}}, $finger_event;
            last;
        }

        my $finger_count = scalar @{$finger_event->{Finger_Array}};
        if ($finger_event->{State}) {
            if ($finger_count == 1) {
                if (not defined $striking_event) {
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

    $striking_hand = $HAND_OUTPUT{$striking_event->{Hand}};
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
    push @Fingering, $fingering;
}

open FINGERS, "> $Finger_Output_Path" or die "Bad abcDF output open";
# print FINGERS join('', @Fingering);
my $fingering_str = '';
my $last_hand = '';
foreach my $fingering (@Fingering) {
    if ($fingering eq 'x') {
        $fingering_str .= $fingering;
        next;
    }

    my $output_fingering = '';
    if ($fingering =~ /([<>])(\d)(,([<>])(\d))?/) {
        my $striking_hand = $1;
        my $striking_finger = $2;
        if (not $fingering_str) {
            $fingering_str = $striking_hand;
            $last_hand = $striking_hand;
        }

        if ($striking_hand ne $last_hand) {
            $output_fingering .= $striking_hand;
            $last_hand = $striking_hand;
        }
        $output_fingering .= $striking_finger;

        my $releasing_hand = '';
        my $releasing_finger = '';
        if ($3) {
            $releasing_hand = $4;
            $releasing_finger = $5;
            $output_fingering .= ',';
            if ($releasing_hand and $releasing_hand ne $last_hand) {
                $output_fingering .= $releasing_hand;
                $last_hand = $releasing_hand;
            }
            $output_fingering .= $releasing_finger;
        }
        $fingering_str .= $output_fingering;
    } else {
        die "Bad fingering";
    }
}
print FINGERS $fingering_str, "\n";
close FINGERS;

print_interleaved_events();

print "MIDI output written to $Midi_Output_Path\n";
print "abcDF output written to $Finger_Output_Path\n";
print "Done Dactylizing.\n\n";
