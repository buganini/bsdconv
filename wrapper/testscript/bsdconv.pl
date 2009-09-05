#!/usr/local/bin/perl
use bsdconv;

$cd=bsdconv::create($ARGV[0]);
if(!defined($cd)){ 
	print bsdconv::error()."\n";
	exit; 
}
while($str=<STDIN>){
print bsdconv::conv($cd, $str);
}
$i=bsdconv::info($cd);
bsdconv::destroy($cd);
print "\n=======Conversion Info=======\n";
for $k (keys %$i){
	print "$k=$i->{$k}\n";
}