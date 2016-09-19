#!/usr/bin/env perl -w
# File: align.pm
# Author: David Randolph
# Date: 1 September 2016
# Purpose: Implement LCS approach to performance-score alignment, as
#          described in Chen, Jang, and Liou's IEEE ICASSPC 2014 paper.
#
use strict;
use English;
my %Length;

sub bigger_of($$) {
    my ($x, $y) = @_;
    return $x if $x > $y;
    return $y;
}

sub LCS($$) {
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

