#!/usr/local/bin/perl
use bsdconv;

$cd=bsdconv::create("big5,ascii::utf-8");
while($str=<STDIN>){
print bsdconv::conv($cd, $str);
}
