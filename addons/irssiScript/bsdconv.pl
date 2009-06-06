use strict;
use bsdconv;

use vars qw($VERSION %IRSSI);
$VERSION = '2009060600';
%IRSSI = (
	authors		=> 'Buganini',
	contact		=> 'buganini@gmail.com',
	name		=> 'bsdconv',
	description	=> 'bsdconv conversion',
	license		=> 'BSD',
	url		=> 'http://github.com/buganini/bsdconv',
	modules		=> 'bsdconv',
);

my $bsdconv_ignore = 0;

sub event_send_text () {
	if($bsdconv_ignore){
		return;
	}
	my ($line, $server_rec, $wi_item_rec) = @_;
	my $conversion = Irssi::settings_get_str('bsdconv');
	my $h = bsdconv::create($conversion);
	my $line=bsdconv::conv($h,$line);
	bsdconv::destroy($h);
	$bsdconv_ignore = 1;
	Irssi::signal_emit('send text', $line,  $server_rec, $wi_item_rec);
	Irssi::signal_stop();
	$bsdconv_ignore = 0;
}

Irssi::signal_add_first('send text', "event_send_text");
Irssi::settings_add_str("misc", "bsdconv", "utf-8::utf-8");
