#!/usr/local/bin/php
<?php
$s=file_get_contents('php://stdin');
$a=bsdconv_create($argv[1]);
echo bsdconv($a,$s);
bsdconv_destroy($a);
?>
