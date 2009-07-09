#!/usr/local/bin/php
<?php
$s=file_get_contents('php://stdin');
$a=bsdconv_create($argv[1]);
if($a===false){
	echo bsdconv_error()."\n";
	exit;
}
echo bsdconv($a,$s);
$i=bsdconv_info($a);
bsdconv_destroy($a);
echo "\n\n=======Conversino Info=======\n";
print_r($i);
?>
