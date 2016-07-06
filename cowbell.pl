use MIDI;
use strict;
use warnings;
my @ride_events = (
  ['text_event',0, 'RIDE ON'],
  ['set_tempo', 0, 450_000], # 1qn = .45 seconds
);
my @events = (
  ['text_event',0, 'MORE COWBELL'],
);

for (1 .. 45) {
  push @ride_events,
    ['note_on' , 0,  9, 56, 127],
    ['note_off', 96,  9, 56, 127],
  ;
}
foreach my $delay (reverse(1..96)) {
  push @events,
    ['note_on' ,      0,  9, 56, 127],
    ['note_off', $delay,  9, 56, 127],
  ;
}

my $ride_track = MIDI::Track->new({ 'events' => \@ride_events });
my $cowbell_track = MIDI::Track->new({ 'events' => \@events });
my $opus = MIDI::Opus->new({'format' => 1,
    'ticks' => 96,
    'tracks' => [ $ride_track, $cowbell_track ]});
$opus->write_to_file( 'cowbell.mid' );
