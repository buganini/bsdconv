use strict;
use bsdconv;

use vars qw($VERSION %IRSSI);
$VERSION = '2009060600';
%IRSSI = (
	authors		=> 'Buganini',
	contact		=> 'buganini@gmail.com',
	name		=> 'bsdcv',
	description	=> 'bsdconv conversion',
	license		=> 'BSD',
	url		=> 'http://github.com/buganini/bsdconv',
	modules		=> 'bsdconv',
);

my $bsdconv_ignore = 0;

sub on_recv () {
	if($bsdconv_ignore){
		return;
	}
	my ($server, $line, $nick, $address) = @_;
	my $conversion = Irssi::settings_get_str('bsdconv_in');
	$conversion =~ s/^\s+|\s+$//g;
	my $h = bsdconv::create($conversion);
	if($conversion eq ''){return;}
	if(!defined($h)){
		Irssi:print(bsdconv::error());
		return;
	}
	my $nline=bsdconv::conv($h,$line);
	my $info=bsdconv::info($h);
	if($info->{'ierr'}){
		$nline=$line;
	}
	bsdconv::destroy($h);
	$bsdconv_ignore = 1;
	Irssi::signal_emit('event privmsg', ($server, $nline, $nick, $address));
	$bsdconv_ignore = 0;
	Irssi::signal_stop();
}

sub on_topic () {
	if($bsdconv_ignore){
		return;
	}
	my ($server_rec) = @_;
	my $conversion = Irssi::settings_get_str('bsdconv_in');
	$conversion =~ s/^\s+|\s+$//g;
	if($conversion eq ''){return;}
	my $h = bsdconv::create($conversion);
	if(!defined($h)){
		Irssi:print(bsdconv::error());
		return;
	}
	my $nline=bsdconv::conv($h,$server_rec->{topic});
	my $info=bsdconv::info($h);
	if($info->{'ierr'}){
		$nline=$server_rec->{topic};
	}
	bsdconv::destroy($h);
	$server_rec->{topic}=$nline;
	$bsdconv_ignore = 1;
	Irssi::signal_emit('channel topic changed', $server_rec);
	$bsdconv_ignore = 0;
	Irssi::signal_stop();
}

sub bsdconv_out () {
	if($bsdconv_ignore){
		return;
	}
	my ($line, $server_rec, $wi_item_rec) = @_;
	my $conversion = Irssi::settings_get_str('bsdconv_out');
	$conversion =~ s/^\s+|\s+$//g;
	if($conversion eq ''){return;}
	my $h = bsdconv::create($conversion);
	if(!defined($h)){
		Irssi:print(bsdconv::error());
		return;
	}
	my $nline=bsdconv::conv($h,$line);
	my $info=bsdconv::info($h);
	if($info->{'ierr'}){
		$nline=$line;
	}
	bsdconv::destroy($h);
	$bsdconv_ignore = 1;
	Irssi::signal_emit('send text', $nline,  $server_rec, $wi_item_rec);
	$bsdconv_ignore = 0;
	Irssi::signal_stop();
}

Irssi::signal_add('event privmsg', "on_recv");
#Irssi::signal_add('channel topic changed', "on_topic");
Irssi::signal_add_first('send text', "bsdconv_out");
Irssi::settings_add_str("misc", "bsdconv_in", "byte:byte");
Irssi::settings_add_str("misc", "bsdconv_out", "byte:byte");
