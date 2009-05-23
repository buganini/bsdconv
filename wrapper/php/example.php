<?php
$s=file_get_contents("in.txt");
$h=bsdconv_create('big5,ascii:chs:utf-8');
$s=bsdconv($h, $s);
bsdconv_destroy($h)
echo $s;
?>
