# -*- coding: utf-8 -*-

import sys
import urllib
from bsdconv import Bsdconv

def bsdconv01(dt):
	dt=dt.lstrip("0").upper()
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt

def bnf(s):
	return ",".join([bsdconv01(x) for x in s.strip().split(" ")])

iotest=[
	["big5:utf-8","\xa5\x5c\x5c\xaf\xe0","功\能"],
	["big5-5c,big5:utf-8","\xa5\x5c\x5c\xaf\xe0","功能"],
	["utf-8:big5-5c,big5","功能","\xa5\x5c\x5c\xaf\xe0"],
	["_cp950:utf-8","\xa5\x5c\xaf\xe0","功能"],
	["utf-8:_cp950,ascii","喆",""],
	["utf-8:_uao250,ascii","喆","\x95\xed"],
	["utf-8:big5,cp950-trans","测试","\xb4\xfa\xb8\xd5"],
	["ascii,3f:ascii","test測試test","test??????test"],
	["ascii,any#0137:ascii","test測試test","test777777test"],
	["utf-8:ascii,3f","test測試test","test??test"],
	["utf-8:ascii,any#38","test測試test","test88test"],
	["utf-8:uao250|_cp950,ascii,3f:utf-8","陶喆測試","陶?穘?試"],
	["utf-8:uao250|_cp950,ascii,sub:utf-8","陶喆測試","陶�穘�試"],
	["cns11643:utf-8","1234\x00\x01\x60\x41\x00\x01\x66\x5cabcd","1234測試abcd"],
	["utf-8:cns11643","1234測試abcd","1234\x00\x01\x60\x41\x00\x01\x66\x5cabcd"],
	["ansi-control,utf-8:split:bsdconv-keyword,bsdconv","a\033[1mb","0161,1B5B316D,0162,"],
	["ascii-named-html-entity:utf-8","&uuml;","ü"],
	["ascii-numeric-html-entity:utf-8","&#x6e2c;&#35430;","測試"],
	["utf-8:ascii-hex-numeric-html-entity","測\n","&#x6E2C;&#x0A;"],
	["utf-8:ascii-dec-numeric-html-entity","測\n","&#28204;&#10;"],
	["utf-8:ascii-named-html-entity","Ç","&Ccedil;"],
	["bsdconv:utf-8","016e2c","測"],
	["bsdconv:utf-8","016e2c,018a66","測試"],
	["utf-8:bsdconv","測\n","016E2C010A"],
	["utf-8:pass","測\n","\x01\x6e\x2c\x01\x0a"],
	["utf-8:raw","測試\n","\x6e\x2c\x8a\x66\x0a"],
	["bsdconv-keyword,utf-8:bsdconv-keyword,bsdconv|bsdconv-keyword,bsdconv:bsdconv-keyword,utf-8","測,試\t测,试\n","測,試\t测,试\n"],
	["byte:byte","\xaa\xbb\xcc\xdd","\xaa\xbb\xcc\xdd"],
	["escape:utf-8","%u6e2c","測"],
	["escape:split:bsdconv-keyword,bsdconv","%u6e2c%e8%a9%a6","016E2C,03E8,03A9,03A6,"],
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
	["ansi-control,byte:big5-defrag:byte,ansi-control|skip,big5:split:bsdconv-keyword,bsdconv","\xaf\033[1m\xe0","0180FD,1B5B316D,"],
	["utf-8:chewing:utf-8","abc測試xyz","abcㄘㄜˋㄕˋxyz"],
	["utf-8:chewing:han-pinyin:utf-8","測試","ce4shi4"],
	["utf-8:kana-phonetic:utf-8","ドラえもん","doraemon"],
	["ascii:alias-from:ascii","BIG5","UAO250"],
	["ascii:alias-from:ascii","UAO250","ASCII,_UAO250"],
	["ascii:alias-from:ascii","LOCALE","UTF-8"],
	["ascii:alias-from:ascii","UTF-8","ASCII,_UTF-8"],
	["ascii:alias-to:ascii","BIG5","CP950"],
	["ascii:alias-to:ascii","CP950","_CP950,ASCII"],
	["utf-8:cns11643:split:bsdconv-keyword,bsdconv","測試","02016041,0201665C,"],
	["bsdconv:unicode:split:bsdconv-keyword,bsdconv","02016041,0201665C","016E2C,018A66,"],
	["utf-8:upper:utf-8","testTEST","TESTTEST"],
	["utf-8:lower:utf-8","testTEST","testtest"],
	["utf-8:full:utf-8","testTEST1234","ｔｅｓｔＴＥＳＴ１２３４"],
	["utf-8:half:utf-8","ｔｅｓｔＴＥＳＴ１２３４","testTEST1234"],
	["utf-8:upsidedown:utf-8","FUNNY","Ⅎ∩ᴎᴎ⅄"],
	["utf-8:unix:utf-8","a\r\nb","a\nb"],
	["utf-8:mac:utf-8","a\r\nb","a\rb"],
	["utf-8:win:utf-8","a\nb","a\r\nb"],
	["utf-8:nl2br:utf-8","a\nb","a<br />b"],
	["utf-8:trim-width#22&ambi-as-wide:utf-8","ˋˊ這是個很長的字串啊啊啊","ˋˊ這是個很長的字串啊"],
	["utf-8:trim-width#22:utf-8","ˋˊ這是個很長的字串啊啊啊","ˋˊ這是個很長的字串啊啊"],
	["utf-8:trim-width#10&ambiguous-as-wide:utf-8","三長兩短ˊˋ3長2短","三長兩短ˊ"],
	["utf-8:zh-strings:utf-8","abd測試efg功能，hij","測試\n功能\n"],
	["utf-8:zhcn:utf-8","測試","测试"],
	["utf-8:zhtw:utf-8","测试之后","測試之后"],
	["utf-8:zhtw:zhtw-words:utf-8","测试之后","測試之後"],
	["utf-8:whitespace-derail:zhtw:zhtw-words:whitespace-rerail:utf-8","之 后","之 後"],
	["utf-8:zh-decomp:zh-comp:utf-8","功夫不好不要艹我","巭孬嫑莪"],
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
	["utf-8:nfkd:utf-8","ăǅⓐ","ăDža"],
	["utf-8:nfkc:utf-8","ăǅⓐ","ăDža"],
	["ascii,any#019644.012F:utf-8","A測B","A附/附/附/B"],
	["utf-8:pass,zh-decomp:insert#after=002c:bsdconv-keyword,bsdconv","不大不要","014E0D,015927,014E0D,018981,"],
	["utf-8:pass#limit=2,zh-decomp:insert#after=002c:bsdconv-keyword,bsdconv","不大不要","014E0D,015927,048D,040107,0476,"],
	["bsdconv:nfd:_nf-order:insert#after=002c:bsdconv-keyword,bsdconv","011e9b,010323","01017F,010323,010307,"],
	["utf-8:_nf-hangul-decomposition:utf-8","가","가"],
	["utf-8:casefold:utf-8","AbяЯßẞ","abяяssss"],
	["utf-8:replace#0142.0143=0132.0133:utf-8","ABCD","A23D"],
	["utf-8:strings#min-len=2:utf-8","aㄎabㄎabcㄉabcd","ab\nabc\nabcd\n"],
	["utf-8:strings#min-len=2&before=0128&after=0129.010a:utf-8","aㄎabㄎabcㄉabcd","(ab)\n(abc)\n(abcd)\n"],
	["utf-8:whitespace-derail:zhtw:zhtw-words:whitespace-rerail:utf-8","之 后","之 後"],
	["fallback-unicode:insert#after=002c:bsdconv-keyword,bsdconv", "\xe8","01E8,"],
	["cp950-uda:insert#after=002c:bsdconv-keyword,bsdconv", "\xfa\x40\xfe\xfe\x8e\x40\xa0\xfe\x81\x40\x8d\xfe\xc6\xa1\xc8\xfe", "01E000,01E310,01E311,01EEB7,01EEB8,01F6B0,01F6B1,01F848,"],
	["_utf-8:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xED\xB0\x80", ""],
	["_utf-8#cesu:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xED\xB0\x80", "01010400,"],
	["_utf-8#loose:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xED\xB0\x80", "01D801,01DC00,"],
	["_utf-8#cesu,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81", "013F,013F,013F,"],
	["_utf-8#cesu,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xB0\x80", "013F,013F,013F,"],
	["_utf-8#cesu,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xe9\x99\x84", "013F,013F,013F,019644,"],
	["_utf-8#cesu,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xB0\x80\xe9\x99\x84", "013F,013F,013F,019644,"],
	["_utf-8#loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xe9\x99\x84", "01D801,019644,"],
	["_utf-8#loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xB0\x80\xe9\x99\x84", "01DC00,019644,"],
	["_utf-8#cesu&loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xe9\x99\x84", "01D801,019644,"],
	["_utf-8#cesu&loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xB0\x80\xe9\x99\x84", "01DC00,019644,"],
	["_utf-8#cesu&loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xED\xA0\x81", "01D801,01D801,"],
	["_utf-8#cesu&loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xB0\x80\xED\xB0\x80", "01DC00,01DC00,"],
	["_utf-8#loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xA0\x81\xED\xA0\x81", "01D801,01D801,"],
	["_utf-8#loose,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xED\xB0\x80\xED\xB0\x80", "01DC00,01DC00,"],
	["_utf-8:insert#after=002c:bsdconv-keyword,bsdconv", "\xf0\x80\x80\xaf", ""],
	["_utf-8#overlong:insert#after=002c:bsdconv-keyword,bsdconv", "\xf0\x80\x80\xaf", "012F,"],
	["_utf-8#super:insert#after=002c:bsdconv-keyword,bsdconv", "\xf8\x80\x80\x80\xaf", ""],
	["_utf-8#super&overlong:insert#after=002c:bsdconv-keyword,bsdconv", "\xf8\x80\x80\x80\xaf", "012F,"],
	["_utf-8#super,ascii,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xc1\xbf,\xe0\x9f\xbf,\xf0\x8f\xbf\xbf,\xf8\x87\xbf\xbf\xbf,\xfc\x83\xbf\xbf\xbf\xbf", "013F,013F,012C,013F,013F,013F,012C,013F,013F,013F,013F,012C,013F,013F,013F,013F,013F,012C,013F,013F,013F,013F,013F,013F,"],
	["_utf-8#super&overlong,ascii,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xc1\xbf,\xe0\x9f\xbf,\xf0\x8f\xbf\xbf,\xf8\x87\xbf\xbf\xbf,\xfc\x83\xbf\xbf\xbf\xbf", "017F,012C,0107FF,012C,01FFFF,012C,011FFFFF,012C,0103FFFFFF,"],
	["_utf-8#overlong,ascii,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xc1\xbf,\xe0\x9f\xbf,\xf0\x8f\xbf\xbf,\xf8\x87\xbf\xbf\xbf,\xfc\x83\xbf\xbf\xbf\xbf", "017F,012C,0107FF,012C,01FFFF,012C,013F,013F,013F,013F,013F,012C,013F,013F,013F,013F,013F,013F,"],
	["_utf-8,ascii,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xc0\x80,\xe0\x80\x80,\xf0\x80\x80\x80,\xf8\x80\x80\x80\x80,\xfc\x80\x80\x80\x80\x80", "013F,013F,012C,013F,013F,013F,012C,013F,013F,013F,013F,012C,013F,013F,013F,013F,013F,012C,013F,013F,013F,013F,013F,013F,"],
	["_utf-8#nul&overlong&super,ascii,3f:insert#after=002c:bsdconv-keyword,bsdconv", "\xc0\x80,\xe0\x80\x80,\xf0\x80\x80\x80,\xf8\x80\x80\x80\x80,\xfc\x80\x80\x80\x80\x80", "0100,012C,0100,012C,0100,012C,0100,012C,0100,"],
]

countertest=[
	["utf-8:width:null","123Б測試",{"FULL":2,"AMBI":1,"HALF":3}],
	["utf-8:count:null","123Б測試",{"COUNT":6}],
	["utf-8:count#blah:null","123Б測試",{"BLAH":6}],
	["utf-8:count#for=lala&for=cjk:null","123Б測a試bc",{"COUNT":2}],
]

passed=True

for c, i, o in iotest:
	p=Bsdconv(c)
	if not p:
		print(Bsdconv.error())
		print("Test failed at %s" % repr([c, i, o]))
		del p
		passed=False
		continue
	r=p.conv(i)
	if o != r:
		print("Test failed at %s" % repr([c, i, o]))
		print("expected(%d): %s" % (len(o), repr(o)))
		print("result(%d): %s" % (len(r), repr(r)))
		passed=False
	del p

for c, d, i in countertest:
	p=Bsdconv(c)
	if not p:
		print(Bsdconv.error())
		print("Test failed at %s" % repr([c, i, o]))
		passed=False
		continue
	p.conv(d)
	r=p.counter()
	for k in i:
		if i[k] != r[k]:
			print("Test failed at %s" % repr([c, d, i]))
			print("expected: %s" % repr(i))
			print("result: %s" % repr(r))
			passed=False
	del p

url=""
f_map=open("tmp/map.txt")
for l in f_map:
	l=l.strip().split("\t")
	if l[0]=="NormalizationTest.txt":
		url=l[1]
		break
nt=open("tmp/NormalizationTest.txt")
toSRC=Bsdconv("bsdconv:insert#after=002c:bsdconv-keyword,bsdconv")
toNFC=Bsdconv("bsdconv:nfc:insert#after=002c:bsdconv-keyword,bsdconv")
toNFD=Bsdconv("bsdconv:nfd:insert#after=002c:bsdconv-keyword,bsdconv")
toNFKC=Bsdconv("bsdconv:nfkc:insert#after=002c:bsdconv-keyword,bsdconv")
toNFKD=Bsdconv("bsdconv:nfkd:insert#after=002c:bsdconv-keyword,bsdconv")
print("Normalization Tests: #"+url)
ln = 0
for l in nt:
	ln += 1
	if not l:
		continue
	if l[0]=="#":
		continue
	if l[0]=="@":
		print("\t"+l.strip())
		continue
	c1,c2,c3,c4,c5,comment=l.strip().split(";",5)
	c1=bnf(c1)
	c2=bnf(c2)
	c3=bnf(c3)
	c4=bnf(c4)
	c5=bnf(c5)

	nftest=[
		#NFC
		[toSRC.conv(c2), toNFC.conv(c1), "c2 == toNFC(c1)"],
		[toNFC.conv(c1), toNFC.conv(c2), "toNFC(c1) == toNFC(c2)"],
		[toNFC.conv(c2), toNFC.conv(c3), "toNFC(c2) == toNFC(c3)"],
		[toSRC.conv(c4), toNFC.conv(c4), "c4 == toNFC(c4)"],
		[toNFC.conv(c4), toNFC.conv(c5), "toNFC(c4) == toNFC(c5)"],

		#NFD
		[toSRC.conv(c3), toNFD.conv(c1), "c3 == toNFD(c1)"],
		[toNFD.conv(c1), toNFD.conv(c2), "toNFD(c1) == toNFD(c2)"],
		[toNFD.conv(c2), toNFD.conv(c3), "toNFD(c2) == toNFD(c3)"],
		[toSRC.conv(c5), toNFD.conv(c4), "c5 == toNFD(c4)"],
		[toNFD.conv(c4), toNFD.conv(c5), "toNFD(c4) == toNFD(c5)"],

		#NFKC
		[toSRC .conv(c4), toNFKC.conv(c1), "c4 == toNFKC(c1)"],
		[toNFKC.conv(c1), toNFKC.conv(c2), "toNFKC(c1) == toNFKC(c2)"],
		[toNFKC.conv(c2), toNFKC.conv(c3), "toNFKC(c2) == toNFKC(c3)"],
		[toNFKC.conv(c3), toNFKC.conv(c4), "toNFKC(c3) == toNFKC(c4)"],
		[toNFKC.conv(c4), toNFKC.conv(c5), "toNFKC(c4) == toNFKC(c5)"],

		#NFKD
		[toSRC .conv(c5), toNFKD.conv(c1)," c5 == toNFKD(c1)"],
		[toNFKD.conv(c1), toNFKD.conv(c2), "toNFKD(c1) == toNFKD(c2)"],
		[toNFKD.conv(c2), toNFKD.conv(c3), "toNFKD(c2) == toNFKD(c3)"],
		[toNFKD.conv(c3), toNFKD.conv(c4), "toNFKD(c3) == toNFKD(c4)"],
		[toNFKD.conv(c4), toNFKD.conv(c5), "toNFKD(c4) == toNFKD(c5)"],
	]
	for a,b,desc in nftest:
		if a!=b:
			print ln, "Failed: ", desc, a, "!=", b, comment

print("Conversion tests finished.")
