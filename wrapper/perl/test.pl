#!/usr/local/bin/perl
use bsdconv;

$cd=bsdconv::create($ARGV[0]);
while($str=<STDIN>){
print bsdconv::conv($cd, $str);
}
bsdconv::destroy($cd);
