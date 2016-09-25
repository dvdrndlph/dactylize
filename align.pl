#!/usr/bin/env perl -w
# File: align.pm
# Author: David Randolph
# Date: 1 September 2016
# Purpose: Implement LCS approach to performance-score alignment, as
#          described in Chen, Jang, and Liou's IEEE ICASSPC 2014 paper.
#          Actually, not so fast. I don't see how LCS would work. This
#          seems to call for Needleman-Wunsch.
#
use strict;
use English;
use Data::Dumper;
use Getopt::Long;
use FindBin;
use MIDI;
use Algorithm::NeedlemanWunsch;
my %Length;

my $ABC2MIDI = "$FindBin::Bin/abcmidi/abc2midi";
my $MIDI_SCORE_FILE = '/tmp/score_to_align.mid';

sub bigger_of($$) {
    my ($x, $y) = @_;
    return $x if $x > $y;
    return $y;
}

sub LCS {
    my ($ax, $by) = @_;
    return $Length{"$ax $by"} if defined $Length{"$ax $by"};

    if ($ax eq '' or $by eq '') {
        $Length{"$ax $by"} = 0;
        return 0;
    }
    my $a = $ax;
    my $b = $by;
    my $x = chop $a;
    my $y = chop $b;
    if ($x eq $y) {
        $Length{"$ax $by"} = LCS($a, $b) + 1;
        return $Length{"$ax $by"};
    } else {
        $Length{"$ax $by"} = bigger_of LCS($ax, $b), LCS($a, $by);
        return $Length{"$ax $by"};
    }
}

sub get_notes {
    my ($file) = @_;
    my $opus = MIDI::Opus->new({from_file => $file});
    my @track = $opus->tracks();
    my @note;
    foreach my $track (@track) {
        my $events_r = $track->events_r();
        my $score_r = MIDI::Score::events_r_to_score_r($events_r);
        foreach my $note (@$score_r) {
            push @note, $note;
        }
    }
    return @note;
}

# MAIN BLOCK
my $Opt = {};
GetOptions($Opt,
    'performance_midi_file=s',
    'score_abc_file=s',
    'fingering_file=s',
    'delta_microseconds=i');

die "Score to which performance should be aligned must be specified"
    if not $Opt->{score_abc_file};
die "Performance must be specified with via the --performance flags"
    if not $Opt->{performance_midi_file};
my $Onset_Delta = 100;
if (defined $Opt->{delta_microseconds}) {
    $Onset_Delta = $Opt->{delta_microseconds};
}

system("$ABC2MIDI $Opt->{score_abc_file} -o $MIDI_SCORE_FILE");
die "Bad abc2midi call" if $CHILD_ERROR;

my @performance_notes = get_notes($Opt->{performance_midi_file});
my @score_notes = get_notes($MIDI_SCORE_FILE);

print Dumper $performance_notes[10];
print Dumper $score_notes[10];
