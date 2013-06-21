# -*- coding: utf-8 -*-

import sys
import bsdconv

iotest=[
	["big5:utf-8","\xa5\x5c\x5c\xaf\xe0","功\能"],
	["big5-5c,big5:utf-8","\xa5\x5c\x5c\xaf\xe0","功能"],
	["utf-8:big5-5c,big5","功能","\xa5\x5c\x5c\xaf\xe0"],
	["_cp950:utf-8","\xa5\x5c\xaf\xe0","功能"],
	["utf-8:_cp950,ascii","喆",""],
	["utf-8:_uao250,ascii","喆","\x95\xed"],
	["utf-8:big5,cp950_trans","测试","\xb4\xfa\xb8\xd5"],
	["ascii,3f:ascii","test測試test","test??????test"],
	["ascii,any#0137:ascii","test測試test","test777777test"],
	["utf-8:ascii,3f","test測試test","test??test"],
	["utf-8:ascii,any#38","test測試test","test88test"],
	["utf-8:uao250|cp950,3f:utf-8","陶喆測試","陶?穘?試"],
	["utf-8:uao250|cp950,sub:utf-8","陶喆測試","陶�穘�試"],
	["cns11643:utf-8","1234\x00\x01\x60\x41\x00\x01\x66\x5cabcd","1234測試abcd"],
	["utf-8:cns11643","1234測試abcd","1234\x00\x01\x60\x41\x00\x01\x66\x5cabcd"],
	["ansi-control,utf-8:split:bsdconv_keyword,bsdconv","a\033[1mb","0161,1B5B316D,0162,"],
	["ascii-named-html-entity:utf-8","&uuml;","ü"],
	["ascii-numeric-html-entity:utf-8","&#x6e2c;&#35430;","測試"],
	["utf-8:ascii-hex-numeric-html-entity","測\n","&#x6E2C;&#x0A;"],
	["utf-8:ascii-dec-numeric-html-entity","測\n","&#28204;&#10;"],
	["utf-8:ascii-named-html-entity","Ç","&Ccedil;"],
	["bsdconv:utf-8","016e2c","測"],
	["bsdconv:utf-8","016e2c,018a66","測試"],
	["utf-8:bsdconv","測\n","016E2C010A"],
	["utf-8:bsdconv_raw","測\n","\x01\x6e\x2c\x01\x0a"],
	["utf-8:raw","測試\n","\x6e\x2c\x8a\x66\x0a"],
	["bsdconv_keyword,utf-8:bsdconv_keyword,bsdconv|bsdconv_keyword,bsdconv:bsdconv_keyword,utf-8","測,試\t测,试\n","測,試\t测,试\n"],
	["byte:byte","\xaa\xbb\xcc\xdd","\xaa\xbb\xcc\xdd"],
	["escape:utf-8","%u6e2c","測"],
	["escape:split:bsdconv_keyword,bsdconv","%u6e2c%e8%a9%a6","016E2C,03E8,03A9,03A6,"],
	["escape:pass#mark&for=unicode,byte|pass#unmark,utf-8:utf-8","%u6e2c%e8%a9%a6","測試"],
	["escape,utf-8:pass#mark&for=unicode,byte|pass#unmark,big5:utf-8","%u6e2c%b8%d5功能","測試功能"],
	["escape,ascii-numeric-html-entity,utf-8:pass#mark&for=unicode,byte|pass#unmark,big5:utf-8","%u6e2c%b8%d5&#x529F;能","測試功能"],
	["escape:pass#mark&for=unicode,byte|pass#unmark,utf-8:utf-8","\\346\\270\\254\\350\\251\\246","測試"],
	["utf-8:ascii,ascii-escaped-unicode","test測試","test\\u6E2C\\u8A66"],
	["utf-8:ascii-html-cns11643-img","測","<img class=\"cns11643_img\" src=\"http://www.cns11643.gov.tw/AIDB/png.do?page=1&code=6041\" />"],
	["utf-8:ascii-html-info","測\n","<a href=\"http://www.cns11643.gov.tw/AIDB/query_general_view.do?page=1&code=6041\"><img src=\"http://www.cns11643.gov.tw/AIDB/png.do?page=1&code=6041\" /></a><a href=\"http://www.fileformat.info/info/unicode/char/0A/index.htm\"><img class=\"unicode_img\" src=\"http://www.unicode.org/cgi-bin/refglyph?24-A\" /></a>"],
	["utf-8:ascii-html-unicode-img","測","<img class=\"unicode_img\" src=\"http://www.unicode.org/cgi-bin/refglyph?24-6E2C\" />"],
	["utf-8:null","blah",""],
	["utf-8:ambiguous-pad:utf-8","БИ 2","Б И  2"],
	["utf-8:ambiguous-unpad:utf-8","Б И  2","БИ 2"],
	["ansi-control,byte:big5-defrag:byte,ansi-control|skip,big5:split:bsdconv_keyword,bsdconv","\xaf\033[1m\xe0","0180FD,1B5B316D,"],
	["utf-8:chewing:utf-8","abc測試xyz","abcㄘㄜˋㄕˋxyz"],
	["utf-8:chewing:han_pinyin:utf-8","測試","ce4[sh]4"],
	["utf-8:kana_phonetic:utf-8","ドラえもん","doraemon"],
	["ascii:from_alias:ascii","BIG5","UAO250"],
	["ascii:from_alias:ascii","UAO250","ASCII,_UAO250"],
	["ascii:from_alias:ascii","LOCALE","UTF-8"],
	["ascii:from_alias:ascii","UTF-8","ASCII,_UTF-8"],
	["ascii:to_alias:ascii","BIG5","CP950"],
	["ascii:to_alias:ascii","CP950","_CP950,ASCII"],
	["utf-8:cns11643:split:bsdconv_keyword,bsdconv","測試","02016041,0201665C,"],
	["bsdconv:unicode:split:bsdconv_keyword,bsdconv","02016041,0201665C","016E2C,018A66,"],
	["utf-8:upper:utf-8","testTEST","TESTTEST"],
	["utf-8:lower:utf-8","testTEST","testtest"],
	["utf-8:full:utf-8","testTEST1234","ｔｅｓｔＴＥＳＴ１２３４"],
	["utf-8:half:utf-8","ｔｅｓｔＴＥＳＴ１２３４","testTEST1234"],
	["utf-8:upsidedown:utf-8","FUNNY","Ⅎ∩ᴎᴎ⅄"],
	["utf-8:unix:utf-8","a\r\nb","a\nb"],
	["utf-8:mac:utf-8","a\r\nb","a\rb"],
	["utf-8:win:utf-8","a\nb","a\r\nb"],
	["utf-8:nl2br:utf-8","a\nb","a<br />b"],
	["utf-8:zh-strings:utf-8","abd測試efg功能，hij","測試\n功能\n"],
	["utf-8:zhcn:utf-8","測試","测试"],
	["utf-8:zhtw:utf-8","测试之后","測試之后"],
	["utf-8:zhtw:zhtw_words:utf-8","测试之后","測試之後"],
	["utf-8:whitespace-derail:zhtw:zhtw_words:whitespace-rerail:utf-8","之 后","之 後"],
	["utf-8:zh_decomp:zh_comp:utf-8","功夫不好不要艹我","巭孬嫑莪"],
	["utf-8:ibm-37","EBCDIC test","\xc5\xc2\xc3\xc4\xc9\xc3\x40\xa3\x85\xa2\xa3"],
	["utf-8:ibm-37|ibm-37:utf-8","EBCDIC test","EBCDIC test"],
	["utf-8:ibm-930|ibm-930:utf-8","ドラえもん","ドラえもん"],
	["utf-8:ibm-933|ibm-933:utf-8","십진법","십진법"],
	["utf-8:ibm-935|ibm-935:utf-8","标准码","标准码"],
	["utf-8:ibm-937|ibm-937:utf-8","編碼表","編碼表"],
	["utf-8:ibm-939|ibm-939:utf-8","ドラえもん","ドラえもん"],
	["utf-8:gb18030|gb18030:utf-8","标准码編碼表ドラえもん","标准码編碼表ドラえもん"],
	["utf-8:ascii,escape#for=unicode&mode=16&prefix=2575","測a試b好","%u6E2Ca%u8A66b%u597D"],
	["utf-8:big5|ascii,byte:ascii,escape#for=byte&mode=hex&prefix=5c78","測a試b好","\\xB4\\xFAa\\xB8\\xD5b\\xA6n"],
	["utf-8:big5|ascii,byte:ascii,escape#for=byte&mode=oct&prefix=5c","測a試b好","\\264\\372a\\270\\325b\\246n"],
	["utf-8:big5,pass#for=unicode&mark|pass#unmark,ascii,byte:ascii,url","測test喆試","%B4%FAtest%u5586%B8%D5"],
	["utf-8:ascii,escape#for=unicode&prefix=2623&mode=10&suffix=3b","測test喆試","&#28204;test&#21894;&#35430;"],
	["utf-8:upper:utf-8","aăǅбᾥⅷⓐ","AĂǄБᾭⅧⒶ"],
	["utf-8:lower:utf-8","AĂǄБᾭⅧⒶ","aăǆбᾥⅷⓐ"],
	["utf-8:nfd:utf-8","ăǅⓐ","ăǅⓐ"],
	["utf-8:nfc:utf-8","ăǅⓐ","ăǅⓐ"],
	["utf-8:nfkd:utf-8","ăǅⓐ","ăDža"],
	["utf-8:nfkc:utf-8","ăǅⓐ","ª̆ǅⓐ"],
]

infotest=[
	["utf-8:width:null","123Б測試",{"full":2,"ambi":1,"half":3}]
]

for c, i, o in iotest:
	p=bsdconv.Bsdconv(c)
	if not p:
		print(bsdconv.error())
		print("Test failed at %s" % repr([c, i, o]))
		del p
		sys.exit()
	r=p.conv(i)
	if o != r:
		print("Test failed at %s" % repr([c, i, o]))
		print("expected(%d): %s" % (len(o), repr(o)))
		print("result(%d): %s" % (len(r), repr(r)))
		del p
		sys.exit()
	del p

for c, d, i in infotest:
	p=bsdconv.Bsdconv(c)
	if not p:
		print(bsdconv.error())
		print("Test failed at %s" % repr([c, i, o]))
		del p
		sys.exit()
	p.conv(d)
	r=p.info()
	for k in i:
		if i[k] != r[k]:
			print("Test failed at %s" % repr([c, d, i]))
			print("expected: %s" % repr(i))
			print("result: %s" % repr(r))
			del p
			sys.exit()
	del p

print("Conversion tests passed.")
