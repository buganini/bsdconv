#Online DEMO
http://cnmc.tw/~buganini/bsdconv.php

#Documentation
http://www.slideshare.net/Buganini/bsdconv

See CODECS/bsdconv-man for description for each codecs

#Compiling & Installation:
    make PREFIX=/usr # default is /usr/local
    sudo make install

#Example:

Convert traditional chinese big5 to simplified chinese utf-8

    bsdconv big5:zhcn:utf-8 in.txt > out.txt
    bsdconv big5:zhcn:utf-8 -i in.txt #in-place

Convert traditional chinese utf-8 to simplified chinese GB2312 with transliteration

    bsdconv utf-8:zhcn:cp936,cp936-trans in.txt > out.txt

Convert simplified chinese to traditional chinese

    bsdconv utf-8:zhtw:zhtw-words:utf-8

And ignoring whitespaces mixed in words

    bsdconv utf-8:whitespace-derail:zhtw:zhtw-words:whitespace-rerail:utf-8

Convert big5 data, traditional chinese to simplified chinese,
CRLF/CR/LF to CRLF, to big5 data, translate simplified chinese words, which are
not in big5, to HTML entities, and uppercase the ascii characters.

    bsdconv big5:zhcn:win:upper:big5,htmlentity in.txt > out.txt

Very useful for migrating MySQL DB from Big5 to UTF-8

    bsdconv htmlentity,big5-5c,big5:utf-8 in.sql > out.sql

Recover from mis-decoding/encoding (mistreated big5 as iso-8859-1 and converted to utf-8)

    bsdconv 'utf-8:iso-8859-1|big5:utf-8'

Decode javascript escaped data (byte/unicode mixed) like %u9644%20

    bsdconv 'escape,byte:unicode,byte|skip,ascii:utf-8'

Generate string for fuzzy comparison

    echo ¼ℌăǅⓐ⁹灣湾ド鬒鬒æß | bsdconv UTF-8:ZH-FUZZY-TW:KANA-PHONETIC:CASEFOLD:NFKD:UTF-8
    1⁄4Hădža9灣灣do鬒鬒æss

Translate text to HTML <IMG />

    bsdconv big5:nl2br:ascii,html-img in.txt > out.htm

Use glyph image from http://www.cns11643.gov.tw

    bsdconv utf-8:ascii,ascii-html-cns11643-img in.txt out.htm

Maintain inter map:

    bsdconv bsdconv-keyword,bsdconv:bsdconv-keyword,utf-8 inter/FOO.txt > edit.tmp
    vi edit.tmp
    bsdconv bsdconv-keyword,utf-8:bsdconv-keyword,bsdconv edit.tmp > inter/FOO.txt

#WINDOWS:
Use mingw with Makefile.win to build it, then copy everythings in build/ to c:\bsdconv\
the path of the executable will be c:\bsdconv\bsdconv.exe

If you want to install to directory other than default path
set BSDCONV_PATH environment variable to your path.

Run setEnvVar.bat as administrator could help you set proper environment variables.

#Donate:
https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=XLGVJ76ZAYKC2
