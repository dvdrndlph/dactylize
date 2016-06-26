#!/usr/bin/env perl -w
# File: dactylizer.pl
# Author: David Rndolph
# Date: 23 June 2016
# Purpose: Process raw input to produce a MIDI file and corresponding abcDF file.
#
use strict;
use Data::Dumper;
use File::Path;

my $Midi_Path = $ARGV[0];
my $Finger_Path = $ARGV[1];
print "Dactylizing your performance. . . .\n";
print "MIDI event file: $Midi_Path\n";
print "Fingering file: $Finger_Path\n";

my $Output_Dir = "./output/cooked";
mkpath $Output_Dir if not -d $Output_Dir;

my @Midi_Event;
open MIDI, "< $Midi_Path" or die "Bad open of $Midi_Path";
# note_on channel=0 note=65 velocity=6 time=904.565069914
while (my $midi_line = <MIDI>) {
    next if not $midi_line =~ /^note_/;
    my @token = split /\s+/, $midi_line;
    my %midi_event = ();
    foreach my $token (@token) {
        next if not $token =~ /=/;
        my ($var, $val) = split '=', $token;
        $midi_event{$var} = $val;
    } 
    push @Midi_Event, \%midi_event;
}
print Dumper \@Midi_Event;

my @Finger_Event;
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
    push @Finger_Event, \%finger_event;
}
print Dumper \@Finger_Event;

# The first (lowest) note on full keyboard is A0, which is MIDI 21.


print "Done Dactylizing.\n\n";
