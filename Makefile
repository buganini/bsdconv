DESTDIR?=
PREFIX?=/usr/local
BSDCONV_PATH?=${PREFIX}

CFLAGS+=-O2 -Wall -D_BSDCONV_INTERNAL -DPREFIX='"${PREFIX}"' -DBSDCONV_PATH='"${BSDCONV_PATH}"'
SHLIBVER=11

UNAME_S=$(shell uname -s)
ifeq (${UNAME_S}, Darwin)
SHLIBNAME=libbsdconv.so
else
SHLIBNAME=libbsdconv.so.${SHLIBVER}
endif

LIBS?=
ifeq (${UNAME_S}, Linux)
LIBS+=-ldl
endif

TODO_FILTERS=
TODO_FILTERS+=ANSI
TODO_FILTERS+=ARABIC
TODO_FILTERS+=ARMENIAN
TODO_FILTERS+=ARROWS
TODO_FILTERS+=BRAILLE
TODO_FILTERS+=BYTE
TODO_FILTERS+=CHEROKEE
TODO_FILTERS+=CJK
TODO_FILTERS+=CNS11643
TODO_FILTERS+=CUNEIFORM
TODO_FILTERS+=CURRENCY
TODO_FILTERS+=CYRILLIC
TODO_FILTERS+=DEVANAGARI
TODO_FILTERS+=EGYPTIAN
TODO_FILTERS+=EMOTICON
TODO_FILTERS+=ETHIOPIC
TODO_FILTERS+=GEORGIAN
TODO_FILTERS+=GREEK
TODO_FILTERS+=HANGUL
TODO_FILTERS+=HEBREW
TODO_FILTERS+=HIRAGANA
TODO_FILTERS+=IPA
TODO_FILTERS+=JAVANESE
TODO_FILTERS+=KANNADA
TODO_FILTERS+=KATAKANA
TODO_FILTERS+=KHMER
TODO_FILTERS+=LAO
TODO_FILTERS+=LATIN
TODO_FILTERS+=LATIN1
TODO_FILTERS+=MAHJONG
TODO_FILTERS+=MALAYALAM
TODO_FILTERS+=MATH
TODO_FILTERS+=MIAO
TODO_FILTERS+=MONGOLIAN
TODO_FILTERS+=MUSIC
TODO_FILTERS+=MYANMAR
TODO_FILTERS+=PHONETIC
TODO_FILTERS+=PRINT
TODO_FILTERS+=PUA
TODO_FILTERS+=PUNCTUATION
TODO_FILTERS+=ROMAN
TODO_FILTERS+=SAMARITAN
TODO_FILTERS+=SINHALA
TODO_FILTERS+=SUNDANESE
TODO_FILTERS+=SYRIAC
TODO_FILTERS+=TAGALOG
TODO_FILTERS+=TAMIL
TODO_FILTERS+=TELUGU
TODO_FILTERS+=THAI
TODO_FILTERS+=TIBETAN
TODO_FILTERS+=TIFINAGH
TODO_FILTERS+=UNICODE
TODO_FILTERS+=YI

TODO_SCORERS=
TODO_SCORERS+=CJK
TODO_SCORERS+=LATIN1

TODO_CODECS_BASIC=
TODO_CODECS_BASIC+=from/00
TODO_CODECS_BASIC+=from/ANSI-CONTROL
TODO_CODECS_BASIC+=from/ANY
TODO_CODECS_BASIC+=from/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC+=from/ASCII-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC+=from/ASCII
TODO_CODECS_BASIC+=from/BIG5-5C
TODO_CODECS_BASIC+=from/BSDCONV
TODO_CODECS_BASIC+=from/BSDCONV-KEYWORD
TODO_CODECS_BASIC+=from/BSDCONV-LOG
TODO_CODECS_BASIC+=from/BYTE
TODO_CODECS_BASIC+=from/CP950-UDA
TODO_CODECS_BASIC+=from/ESCAPE
TODO_CODECS_BASIC+=from/FALLBACK-UNICODE
TODO_CODECS_BASIC+=from/HEX
TODO_CODECS_BASIC+=from/PASS
TODO_CODECS_BASIC+=from/UTF-16BE
TODO_CODECS_BASIC+=from/UTF-16LE
TODO_CODECS_BASIC+=from/UTF-32BE
TODO_CODECS_BASIC+=from/UTF-32LE
TODO_CODECS_BASIC+=from/_CP1251
TODO_CODECS_BASIC+=from/_CP1252
TODO_CODECS_BASIC+=from/_CP1253
TODO_CODECS_BASIC+=from/_CP1254
TODO_CODECS_BASIC+=from/_CP1255
TODO_CODECS_BASIC+=from/_CP1256
TODO_CODECS_BASIC+=from/_CP1257
TODO_CODECS_BASIC+=from/_CP1258
TODO_CODECS_BASIC+=from/_CP874
TODO_CODECS_BASIC+=from/_CP932
TODO_CODECS_BASIC+=from/_CP949
TODO_CODECS_BASIC+=from/_GB18030
TODO_CODECS_BASIC+=from/_GBK
TODO_CODECS_BASIC+=from/_HKSCS2004
TODO_CODECS_BASIC+=from/_ISO-8859-1
TODO_CODECS_BASIC+=from/_JIS
TODO_CODECS_BASIC+=from/_JIS0201
TODO_CODECS_BASIC+=from/_JIS0208
TODO_CODECS_BASIC+=from/_JIS0212
TODO_CODECS_BASIC+=from/_SHIFT-JIS
TODO_CODECS_BASIC+=from/_UAO250
TODO_CODECS_BASIC+=from/_UTF-8
TODO_CODECS_BASIC+=inter/ALIAS-FILTER
TODO_CODECS_BASIC+=inter/ALIAS-FROM
TODO_CODECS_BASIC+=inter/ALIAS-INTER
TODO_CODECS_BASIC+=inter/ALIAS-TO
TODO_CODECS_BASIC+=inter/AMBIGUOUS-PAD
TODO_CODECS_BASIC+=inter/AMBIGUOUS-UNPAD
TODO_CODECS_BASIC+=inter/ASCIIFOLD
TODO_CODECS_BASIC+=inter/BIG5-DEFRAG
TODO_CODECS_BASIC+=inter/CASEFOLD
TODO_CODECS_BASIC+=inter/COUNT
TODO_CODECS_BASIC+=inter/FULL
TODO_CODECS_BASIC+=inter/HALF
TODO_CODECS_BASIC+=inter/HANJA
TODO_CODECS_BASIC+=inter/INSERT
TODO_CODECS_BASIC+=inter/KANA-PHONETIC
TODO_CODECS_BASIC+=inter/KANJI
TODO_CODECS_BASIC+=inter/LOWER
TODO_CODECS_BASIC+=inter/MAC
TODO_CODECS_BASIC+=inter/NL2BR
TODO_CODECS_BASIC+=inter/NULL
TODO_CODECS_BASIC+=inter/PASS
TODO_CODECS_BASIC+=inter/REPLACE
TODO_CODECS_BASIC+=inter/SCORE
TODO_CODECS_BASIC+=inter/SCORE-TRAIN
TODO_CODECS_BASIC+=inter/STRINGS
TODO_CODECS_BASIC+=inter/SUB
TODO_CODECS_BASIC+=inter/SUPER
TODO_CODECS_BASIC+=inter/TRIM-WIDTH
TODO_CODECS_BASIC+=inter/UNIX
TODO_CODECS_BASIC+=inter/UPPER
TODO_CODECS_BASIC+=inter/UPSIDEDOWN
TODO_CODECS_BASIC+=inter/WHITESPACE-DERAIL
TODO_CODECS_BASIC+=inter/WHITESPACE-RERAIL
TODO_CODECS_BASIC+=inter/WIDTH
TODO_CODECS_BASIC+=inter/WIN
TODO_CODECS_BASIC+=inter/ZHCN
TODO_CODECS_BASIC+=inter/ZHTW
TODO_CODECS_BASIC+=inter/ZHTW-WORDS
TODO_CODECS_BASIC+=inter/ZH-FUZZY-TW
TODO_CODECS_BASIC+=inter/ZH-FUZZY-CN
TODO_CODECS_BASIC+=inter/_NFC
TODO_CODECS_BASIC+=inter/_NFC-MAP
TODO_CODECS_BASIC+=inter/_NFD
TODO_CODECS_BASIC+=inter/_NFKD
TODO_CODECS_BASIC+=inter/_NF-HANGUL-COMPOSITION
TODO_CODECS_BASIC+=inter/_NF-HANGUL-DECOMPOSITION
TODO_CODECS_BASIC+=inter/_NF-ORDER
TODO_CODECS_BASIC+=to/00
TODO_CODECS_BASIC+=to/ANY
TODO_CODECS_BASIC+=to/ASCII-HTML-INFO
TODO_CODECS_BASIC+=to/ASCII-HTML-UNICODE-IMG
TODO_CODECS_BASIC+=to/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC+=to/ASCII
TODO_CODECS_BASIC+=to/BIG5-5C
TODO_CODECS_BASIC+=to/BSDCONV
TODO_CODECS_BASIC+=to/BSDCONV-KEYWORD
TODO_CODECS_BASIC+=to/BSDCONV-OUTPUT
TODO_CODECS_BASIC+=to/BSDCONV-LOG
TODO_CODECS_BASIC+=to/BYTE
TODO_CODECS_BASIC+=to/_CP1251
TODO_CODECS_BASIC+=to/_CP1252
TODO_CODECS_BASIC+=to/_CP1253
TODO_CODECS_BASIC+=to/_CP874
TODO_CODECS_BASIC+=to/_CP936
TODO_CODECS_BASIC+=to/CP936-TRANS
TODO_CODECS_BASIC+=to/_CP949
TODO_CODECS_BASIC+=to/_CP950
TODO_CODECS_BASIC+=to/CP950-TRANS
TODO_CODECS_BASIC+=to/ESCAPE
TODO_CODECS_BASIC+=to/_GB18030
TODO_CODECS_BASIC+=to/_GBK
TODO_CODECS_BASIC+=to/HEX
TODO_CODECS_BASIC+=to/_ISO-8859-1
TODO_CODECS_BASIC+=to/_JIS0212
TODO_CODECS_BASIC+=to/_SHIFT-JIS
TODO_CODECS_BASIC+=to/NULL
TODO_CODECS_BASIC+=to/PASS
TODO_CODECS_BASIC+=to/RAW
TODO_CODECS_BASIC+=to/_UAO250
TODO_CODECS_BASIC+=to/UCS-2BE
TODO_CODECS_BASIC+=to/UCS-2LE
TODO_CODECS_BASIC+=to/UTF-16BE
TODO_CODECS_BASIC+=to/UTF-16LE
TODO_CODECS_BASIC+=to/UTF-32BE
TODO_CODECS_BASIC+=to/UTF-32LE
TODO_CODECS_BASIC+=to/_UTF-8

TODO_CODECS_CHINESE=
TODO_CODECS_CHINESE+=from/CCCII
TODO_CODECS_CHINESE+=from/_BIG5E
TODO_CODECS_CHINESE+=from/_BIG5-2003
TODO_CODECS_CHINESE+=from/_BIG5-ETEN
TODO_CODECS_CHINESE+=from/_CP936
TODO_CODECS_CHINESE+=from/_CP950
TODO_CODECS_CHINESE+=from/_HKSCS1999
TODO_CODECS_CHINESE+=from/_HKSCS2001
TODO_CODECS_CHINESE+=from/_GB2312
TODO_CODECS_CHINESE+=from/_UAO241
TODO_CODECS_CHINESE+=inter/ZH-BONUS
TODO_CODECS_CHINESE+=inter/ZH-BONUS-PHRASE
TODO_CODECS_CHINESE+=to/CCCII
TODO_CODECS_CHINESE+=to/_GB2312
TODO_CODECS_CHINESE+=to/_UAO241

TODO_CODECS_EBCDIC=
TODO_CODECS_EBCDIC+=from/IBM-37
TODO_CODECS_EBCDIC+=from/IBM-930
TODO_CODECS_EBCDIC+=from/IBM-933
TODO_CODECS_EBCDIC+=from/IBM-935
TODO_CODECS_EBCDIC+=from/IBM-937
TODO_CODECS_EBCDIC+=from/IBM-939
TODO_CODECS_EBCDIC+=to/IBM-37
TODO_CODECS_EBCDIC+=to/IBM-930
TODO_CODECS_EBCDIC+=to/IBM-933
TODO_CODECS_EBCDIC+=to/IBM-935
TODO_CODECS_EBCDIC+=to/IBM-937
TODO_CODECS_EBCDIC+=to/IBM-939

all: libbsdconv bsdconv-mktable meta bsdconv-man bsdconv-completion bsdconv filters scorers codecs

alias:
	python tools/mkalias.py modules/filter/alias modules/inter/ALIAS-FILTER.txt
	python tools/mkalias.py modules/from/alias modules/inter/ALIAS-FROM.txt
	@printf "014C,014F,0143,0141,014C,0145\t?\n" >> modules/inter/ALIAS-FROM.txt
	python tools/mkalias.py modules/inter/alias modules/inter/ALIAS-INTER.txt
	@printf "0155,014E,0149,0143,014F,0144,0145\t?\n" >> modules/inter/ALIAS-INTER.txt
	python tools/mkalias.py modules/to/alias modules/inter/ALIAS-TO.txt
	@printf "014C,014F,0143,0141,014C,0145\t?\n" >> modules/inter/ALIAS-TO.txt

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/share/bsdconv/filter
	mkdir -p build/share/bsdconv/scorer
	mkdir -p build/share/bsdconv/from
	mkdir -p build/share/bsdconv/inter
	mkdir -p build/share/bsdconv/to

installdir:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${PREFIX}/lib
	mkdir -p ${DESTDIR}${PREFIX}/include
	mkdir -p ${DESTDIR}${PREFIX}/share/bsdconv/filter
	mkdir -p ${DESTDIR}${PREFIX}/share/bsdconv/scorer
	mkdir -p ${DESTDIR}${PREFIX}/share/bsdconv/from
	mkdir -p ${DESTDIR}${PREFIX}/share/bsdconv/inter
	mkdir -p ${DESTDIR}${PREFIX}/share/bsdconv/to

libbsdconv: src/libbsdconv*.c src/bsdconv.h | builddir
	$(CC) ${CFLAGS} src/libbsdconv.c -fPIC -shared -o build/lib/${SHLIBNAME} ${LIBS}

bsdconv: libbsdconv meta src/bsdconv.h src/bsdconv.c | builddir
	$(CC) ${CFLAGS} src/bsdconv.c -L./build/lib/ -o build/bin/bsdconv -lbsdconv ${LIBS}

bsdconv-mktable: src/bsdconv.h src/bsdconv-mktable.c | builddir
	$(CC) ${CFLAGS} -DUSE_FMALLOC src/libfmalloc.c src/bsdconv-mktable.c -o build/bin/bsdconv-mktable

bsdconv-man: libbsdconv meta src/bsdconv.h src/bsdconv.c | builddir
	$(CC) ${CFLAGS} src/bsdconv-man.c -L./build/lib/ -o build/bin/bsdconv-man -lbsdconv ${LIBS}

bsdconv-completion: libbsdconv meta src/bsdconv.h src/bsdconv-completion.c | builddir
	$(CC) ${CFLAGS} src/bsdconv-completion.c -L./build/lib -o build/bin/bsdconv-completion -lbsdconv ${LIBS}

bsdconv-dbg: libbsdconv src/libbsdconv.c src/bsdconv.h src/bsdconv-dbg.c | builddir
	$(CC) ${CFLAGS} src/libbsdconv.c src/bsdconv-dbg.c -o build/bin/bsdconv-dbg ${LIBS}

filters: libbsdconv meta | builddir
	for item in ${TODO_FILTERS} ; do \
		echo Build filter $${item}.so ; \
		$(CC) ${CFLAGS} modules/filter/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/filter/$${item}.so -lbsdconv ${LIBS} ; \
		if [ -e modules/filter/$${item}.man ]; then cp modules/filter/$${item}.man ./build/share/bsdconv/filter/$${item}.man ; fi ; \
	done

scorers: libbsdconv meta | builddir
	for item in ${TODO_SCORERS} ; do \
		echo Build scorer $${item}.so ; \
		$(CC) ${CFLAGS} modules/scorer/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/scorer/$${item}.so -lbsdconv ${LIBS} ; \
		if [ -e modules/scorer/$${item}.man ]; then cp modules/scorer/$${item}.man ./build/share/bsdconv/scorer/$${item}.man ; fi ; \
	done

codecs_basic: bsdconv-mktable libbsdconv meta | builddir
	for item in ${TODO_CODECS_BASIC} ; do \
		./build/bin/bsdconv-mktable modules/$${item}.txt ./build/share/bsdconv/$${item} ; \
		if [ -e modules/$${item}.man ]; then cp modules/$${item}.man ./build/share/bsdconv/$${item}.man ; fi ; \
		if [ -e modules/$${item}.c ]; then echo Build $${item}.so; $(CC) ${CFLAGS} modules/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; fi ; \
	done

codecs_chinese: bsdconv-mktable libbsdconv meta | builddir
	for item in ${TODO_CODECS_CHINESE} ; do \
		./build/bin/bsdconv-mktable modules/$${item}.txt ./build/share/bsdconv/$${item} ; \
		if [ -e modules/$${item}.man ]; then cp modules/$${item}.man ./build/share/bsdconv/$${item}.man ; fi ; \
		if [ -e modules/$${item}.c ]; then echo Build $${item}.so; $(CC) ${CFLAGS} modules/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; fi ; \
	done

codecs_ebcdic: bsdconv-mktable libbsdconv meta | builddir
	for item in ${TODO_CODECS_EBCDIC} ; do \
		./build/bin/bsdconv-mktable modules/$${item}.txt ./build/share/bsdconv/$${item} ; \
		if [ -e modules/$${item}.man ]; then cp modules/$${item}.man ./build/share/bsdconv/$${item}.man ; fi ; \
		if [ -e modules/$${item}.c ]; then echo Build $${item}.so; $(CC) ${CFLAGS} modules/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; fi ; \
	done

codecs: codecs_basic codecs_chinese codecs_ebcdic

meta: libbsdconv
	if [ ${SHLIBNAME} != libbsdconv.so ]; then \
		ln -sf libbsdconv.so.${SHLIBVER} build/lib/libbsdconv.so ; \
	fi
	cp src/bsdconv.h build/include
	cp modules/from/alias build/share/bsdconv/from/alias
	cp modules/inter/alias build/share/bsdconv/inter/alias
	cp modules/to/alias build/share/bsdconv/to/alias
	cp modules/filter/alias build/share/bsdconv/filter/alias

clean:
	rm -rf build
	rm -rf testsuite/api
	rm -rf tmp

install: install_main install_filters install_scorers install_basic install_chinese install_ebcdic

install_main: | installdir
	install -m 555 build/bin/bsdconv ${DESTDIR}${PREFIX}/bin
	install -m 555 build/bin/bsdconv-man ${DESTDIR}${PREFIX}/bin
	install -m 555 build/bin/bsdconv-mktable ${DESTDIR}${PREFIX}/bin
	install -m 555 build/bin/bsdconv-completion ${DESTDIR}${PREFIX}/bin
	install -m 444 build/include/bsdconv.h ${DESTDIR}${PREFIX}/include
	install -m 444 build/lib/${SHLIBNAME} ${DESTDIR}${PREFIX}/lib
	install -m 444 build/share/bsdconv/from/alias ${DESTDIR}${PREFIX}/share/bsdconv/from/alias
	install -m 444 build/share/bsdconv/inter/alias ${DESTDIR}${PREFIX}/share/bsdconv/inter/alias
	install -m 444 build/share/bsdconv/to/alias ${DESTDIR}${PREFIX}/share/bsdconv/to/alias
	install -m 444 build/share/bsdconv/filter/alias ${DESTDIR}${PREFIX}/share/bsdconv/filter/alias
	if [ ${SHLIBNAME} != libbsdconv.so ]; then \
		ln -sf libbsdconv.so.${SHLIBVER} ${DESTDIR}${PREFIX}/lib/libbsdconv.so ; \
	fi

install_filters: | installdir
	for item in ${TODO_FILTERS} ; do \
		install -m 444 build/share/bsdconv/filter/$${item}.so ${DESTDIR}${PREFIX}/share/bsdconv/filter/$${item}.so ; \
		if [ -e build/share/bsdconv/filter/$${item}.man ]; then install -m 444 build/share/bsdconv/filter/$${item}.man ${DESTDIR}${PREFIX}/share/bsdconv/filter/$${item}.man ; fi ; \
	done

install_scorers: | installdir
	for item in ${TODO_SCORERS} ; do \
		install -m 444 build/share/bsdconv/scorer/$${item}.so ${DESTDIR}${PREFIX}/share/bsdconv/scorer/$${item}.so ; \
		if [ -e build/share/bsdconv/scorer/$${item}.man ]; then install -m 444 build/share/bsdconv/scorer/$${item}.man ${DESTDIR}${PREFIX}/share/bsdconv/scorer/$${item}.man ; fi ; \
	done

install_basic: | installdir
	for item in ${TODO_CODECS_BASIC} ; do \
		install -m 444 build/share/bsdconv/$${item} ${DESTDIR}${PREFIX}/share/bsdconv/$${item} ; \
		if [ -e build/share/bsdconv/$${item}.man ]; then install -m 444 build/share/bsdconv/$${item}.man ${DESTDIR}${PREFIX}/share/bsdconv/$${item}.man ; fi ; \
		if [ -e build/share/bsdconv/$${item}.so ]; then install -m 444 build/share/bsdconv/$${item}.so ${DESTDIR}${PREFIX}/share/bsdconv/$${item}.so ; fi ; \
	done

install_chinese: | installdir
	for item in ${TODO_CODECS_CHINESE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${DESTDIR}${PREFIX}/share/bsdconv/$${item} ; \
		if [ -e build/share/bsdconv/$${item}.man ]; then install -m 444 build/share/bsdconv/$${item}.man ${DESTDIR}${PREFIX}/share/bsdconv/$${item}.man ; fi ; \
		if [ -e build/share/bsdconv/$${item}.so ]; then install -m 444 build/share/bsdconv/$${item}.so ${DESTDIR}${PREFIX}/share/bsdconv/$${item}.so ; fi ; \
	done

install_ebcdic: | installdir
	for item in ${TODO_CODECS_EBCDIC} ; do \
		install -m 444 build/share/bsdconv/$${item} ${DESTDIR}${PREFIX}/share/bsdconv/$${item} ; \
		if [ -e build/share/bsdconv/$${item}.man ]; then install -m 444 build/share/bsdconv/$${item}.man ${DESTDIR}${PREFIX}/share/bsdconv/$${item}.man ; fi ; \
		if [ -e build/share/bsdconv/$${item}.so ]; then install -m 444 build/share/bsdconv/$${item}.so ${DESTDIR}${PREFIX}/share/bsdconv/$${item}.so ; fi ; \
	done

build_doc: build_doc_tex build_doc_sphinx

build_doc_tex:
	xelatex -synctex=1 -interaction=nonstopmode doc/tex/bsdconv.tex //1-pass, for TOC
	xelatex -synctex=1 -interaction=nonstopmode doc/tex/bsdconv.tex //2-pass

build_doc_sphinx:
	$(MAKE) -C doc html

plist:
	@echo bin/bsdconv
	@echo bin/bsdconv-completion
	@echo bin/bsdconv-man
	@echo bin/bsdconv-mktable
	@echo include/bsdconv.h
	@echo lib/libbsdconv.so
	@echo lib/${SHLIBNAME}
	@echo %%DATADIR%%/from/alias
	@echo %%DATADIR%%/inter/alias
	@echo %%DATADIR%%/to/alias
	@for item in ${TODO_CODECS_BASIC} ; do \
		echo %%DATADIR%%/$${item} ; \
		if [ -e modules/$${item}.man ]; then echo %%DATADIR%%/$${item}.man ; fi ; \
		if [ -e modules/$${item}.c ]; then echo %%DATADIR%%/$${item}.so ; fi ; \
	done
	@for item in ${TODO_CODECS_CHINESE} ; do \
		echo %%CHINESE%%%%DATADIR%%/$${item} ; \
		if [ -e modules/$${item}.man ]; then echo %%CHINESE%%%%DATADIR%%/$${item}.man ; fi ; \
		if [ -e modules/$${item}.c ]; then echo %%CHINESE%%%%DATADIR%%/$${item}.so ; fi ; \
	done
	@for item in ${TODO_CODECS_EBCDIC} ; do \
		echo %%EBCDIC%%%%DATADIR%%/$${item} ; \
		if [ -e modules/$${item}.man ]; then echo %%EBCDIC%%%%DATADIR%%/$${item}.man ; fi ; \
		if [ -e modules/$${item}.c ]; then echo %%EBCDIC%%%%DATADIR%%/$${item}.so ; fi ; \
	done

UnicodeData=ftp://ftp.unicode.org/Public/13.0.0/ucd/UnicodeData.txt
DerivedNormalizationProps=ftp://ftp.unicode.org/Public/13.0.0/ucd/DerivedNormalizationProps.txt
EastAsianWidth=ftp://ftp.unicode.org/Public/13.0.0/ucd/EastAsianWidth.txt
NormalizationTest=ftp://ftp.unicode.org/Public/13.0.0/ucd/NormalizationTest.txt
SpecialCasing=ftp://ftp.unicode.org/Public/13.0.0/ucd/SpecialCasing.txt
CaseFolding=ftp://ftp.unicode.org/Public/13.0.0/ucd/CaseFolding.txt
Blocks=ftp://ftp.unicode.org/Public/13.0.0/ucd/Blocks.txt
fetch:
	@mkdir -p tmp
	@if [ ! -e tmp/UnicodeData.txt ]; then \
		wget -O tmp/UnicodeData.txt ${UnicodeData}; \
	fi ;
	@if [ ! -e tmp/DerivedNormalizationProps.txt ]; then \
		wget -O tmp/DerivedNormalizationProps.txt ${DerivedNormalizationProps}; \
	fi ;
	@if [ ! -e tmp/EastAsianWidth.txt ]; then \
		wget -O tmp/EastAsianWidth.txt ${EastAsianWidth}; \
	fi ;
	@if [ ! -e tmp/NormalizationTest.txt ]; then \
		wget -O tmp/NormalizationTest.txt ${NormalizationTest}; \
	fi ;
	@if [ ! -e tmp/SpecialCasing.txt ]; then \
		wget -O tmp/SpecialCasing.txt ${SpecialCasing}; \
	fi ;
	@if [ ! -e tmp/CaseFolding.txt ]; then \
		wget -O tmp/CaseFolding.txt ${CaseFolding}; \
	fi ;
	@if [ ! -e tmp/Blocks.txt ]; then \
		wget -O tmp/Blocks.txt ${Blocks}; \
	fi ;
	@cat /dev/null > tmp/map.txt
	@echo "UnicodeData.txt	${UnicodeData}" >> tmp/map.txt
	@echo "DerivedNormalizationProps.txt	${DerivedNormalizationProps}" >> tmp/map.txt
	@echo "EastAsianWidth.txt	${EastAsianWidth}" >> tmp/map.txt
	@echo "NormalizationTest.txt	${NormalizationTest}" >> tmp/map.txt
	@echo "SpecialCasing.txt	${SpecialCasing}" >> tmp/map.txt
	@echo "CaseFolding.txt	${CaseFolding}" >> tmp/map.txt
	@echo "Blocks.txt	${Blocks}" >> tmp/map.txt

test: fetch
	@python testsuite/conversion.py
	@$(CC) ${CFLAGS} -Isrc/ testsuite/api.c -L./build/lib/ -o testsuite/api -lbsdconv ${LIBS}
	@./testsuite/api

gen: unicode_gen bonus

unicode_gen: fetch
	python tools/unicode_gen.py

bonus:
	python tools/mkbonus.py modules/src/ZH-BONUS.txt modules/inter/ZH-BONUS.txt modules/inter/ZH-BONUS-PHRASE.txt
