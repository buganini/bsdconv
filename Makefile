PREFIX?=/usr/local
BSDCONV_PATH?=${PREFIX}
CFLAGS+=-Wall -DPREFIX='"${PREFIX}"' -DBSDCONV_PATH='"${BSDCONV_PATH}"'
SHLIBVER=10

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

TODO_CODECS_BASIC_TABLE=
TODO_CODECS_BASIC_TABLE+=from/00
TODO_CODECS_BASIC_TABLE+=from/ANSI-CONTROL
TODO_CODECS_BASIC_TABLE+=from/ANY
TODO_CODECS_BASIC_TABLE+=from/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=from/ASCII-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=from/ASCII
TODO_CODECS_BASIC_TABLE+=from/BIG5-5C
TODO_CODECS_BASIC_TABLE+=from/BSDCONV
TODO_CODECS_BASIC_TABLE+=from/BSDCONV_KEYWORD
TODO_CODECS_BASIC_TABLE+=from/BSDCONV_LOG
TODO_CODECS_BASIC_TABLE+=from/BYTE
TODO_CODECS_BASIC_TABLE+=from/ESCAPE
TODO_CODECS_BASIC_TABLE+=from/PASS
TODO_CODECS_BASIC_TABLE+=from/UTF-16BE
TODO_CODECS_BASIC_TABLE+=from/UTF-16LE
TODO_CODECS_BASIC_TABLE+=from/UTF-32BE
TODO_CODECS_BASIC_TABLE+=from/UTF-32LE
TODO_CODECS_BASIC_TABLE+=from/_CP1251
TODO_CODECS_BASIC_TABLE+=from/_CP1252
TODO_CODECS_BASIC_TABLE+=from/_CP1253
TODO_CODECS_BASIC_TABLE+=from/_CP874
TODO_CODECS_BASIC_TABLE+=from/_CP949
TODO_CODECS_BASIC_TABLE+=from/_GB18030
TODO_CODECS_BASIC_TABLE+=from/_GBK
TODO_CODECS_BASIC_TABLE+=from/_ISO-8859-1
TODO_CODECS_BASIC_TABLE+=from/_JIS0212
TODO_CODECS_BASIC_TABLE+=from/_SHIFT-JIS
TODO_CODECS_BASIC_TABLE+=from/_UAO250
TODO_CODECS_BASIC_TABLE+=from/_UTF-8
TODO_CODECS_BASIC_TABLE+=inter/AMBIGUOUS-PAD
TODO_CODECS_BASIC_TABLE+=inter/AMBIGUOUS-UNPAD
TODO_CODECS_BASIC_TABLE+=inter/BIG5-DEFRAG
TODO_CODECS_BASIC_TABLE+=inter/FROM_ALIAS
TODO_CODECS_BASIC_TABLE+=inter/FULL
TODO_CODECS_BASIC_TABLE+=inter/HALF
TODO_CODECS_BASIC_TABLE+=inter/INTER_ALIAS
TODO_CODECS_BASIC_TABLE+=inter/KANA_PHONETIC
TODO_CODECS_BASIC_TABLE+=inter/LOWER
TODO_CODECS_BASIC_TABLE+=inter/MAC
TODO_CODECS_BASIC_TABLE+=inter/NOBOM
TODO_CODECS_BASIC_TABLE+=inter/NL2BR
TODO_CODECS_BASIC_TABLE+=inter/SCORE
TODO_CODECS_BASIC_TABLE+=inter/SCORE_TRAIN
TODO_CODECS_BASIC_TABLE+=inter/SPLIT
TODO_CODECS_BASIC_TABLE+=inter/TO_ALIAS
TODO_CODECS_BASIC_TABLE+=inter/TRIM-WIDTH
TODO_CODECS_BASIC_TABLE+=inter/UNIX
TODO_CODECS_BASIC_TABLE+=inter/UPPER
TODO_CODECS_BASIC_TABLE+=inter/UPSIDEDOWN
TODO_CODECS_BASIC_TABLE+=inter/WHITESPACE-DERAIL
TODO_CODECS_BASIC_TABLE+=inter/WHITESPACE-RERAIL
TODO_CODECS_BASIC_TABLE+=inter/WIDTH
TODO_CODECS_BASIC_TABLE+=inter/WIN
TODO_CODECS_BASIC_TABLE+=inter/ZH-STRINGS
TODO_CODECS_BASIC_TABLE+=inter/ZHCN
TODO_CODECS_BASIC_TABLE+=inter/ZHTW
TODO_CODECS_BASIC_TABLE+=inter/ZHTW_WORDS
TODO_CODECS_BASIC_TABLE+=inter/ZH_FUZZY_TW
TODO_CODECS_BASIC_TABLE+=inter/ZH_FUZZY_CN
TODO_CODECS_BASIC_TABLE+=to/00
TODO_CODECS_BASIC_TABLE+=to/ANY
TODO_CODECS_BASIC_TABLE+=to/ASCII-HTML-CNS11643-IMG
TODO_CODECS_BASIC_TABLE+=to/ASCII-HTML-INFO
TODO_CODECS_BASIC_TABLE+=to/ASCII-HTML-UNICODE-IMG
TODO_CODECS_BASIC_TABLE+=to/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=to/ASCII
TODO_CODECS_BASIC_TABLE+=to/BIG5-5C
TODO_CODECS_BASIC_TABLE+=to/BSDCONV
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_KEYWORD
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_LOG
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_STDOUT
TODO_CODECS_BASIC_TABLE+=to/BYTE
TODO_CODECS_BASIC_TABLE+=to/_CP1251
TODO_CODECS_BASIC_TABLE+=to/_CP1252
TODO_CODECS_BASIC_TABLE+=to/_CP1253
TODO_CODECS_BASIC_TABLE+=to/_CP874
TODO_CODECS_BASIC_TABLE+=to/_CP936
TODO_CODECS_BASIC_TABLE+=to/CP936_TRANS
TODO_CODECS_BASIC_TABLE+=to/_CP949
TODO_CODECS_BASIC_TABLE+=to/_CP950
TODO_CODECS_BASIC_TABLE+=to/CP950_TRANS
TODO_CODECS_BASIC_TABLE+=to/ESCAPE
TODO_CODECS_BASIC_TABLE+=to/_GB18030
TODO_CODECS_BASIC_TABLE+=to/_GBK
TODO_CODECS_BASIC_TABLE+=to/_ISO-8859-1
TODO_CODECS_BASIC_TABLE+=to/_JIS0212
TODO_CODECS_BASIC_TABLE+=to/_SHIFT-JIS
TODO_CODECS_BASIC_TABLE+=to/NULL
TODO_CODECS_BASIC_TABLE+=to/PASS
TODO_CODECS_BASIC_TABLE+=to/RAW
TODO_CODECS_BASIC_TABLE+=to/_UAO250
TODO_CODECS_BASIC_TABLE+=to/UCS-2BE
TODO_CODECS_BASIC_TABLE+=to/UCS-2LE
TODO_CODECS_BASIC_TABLE+=to/UTF-16BE
TODO_CODECS_BASIC_TABLE+=to/UTF-16LE
TODO_CODECS_BASIC_TABLE+=to/UTF-32BE
TODO_CODECS_BASIC_TABLE+=to/UTF-32LE
TODO_CODECS_BASIC_TABLE+=to/_UTF-8

TODO_CODECS_BASIC_CALLBACK=
TODO_CODECS_BASIC_CALLBACK+=from/ANSI-CONTROL
TODO_CODECS_BASIC_CALLBACK+=from/ANY
TODO_CODECS_BASIC_CALLBACK+=from/ASCII-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC_CALLBACK+=from/BSDCONV
TODO_CODECS_BASIC_CALLBACK+=from/BSDCONV_LOG
TODO_CODECS_BASIC_CALLBACK+=from/ESCAPE
TODO_CODECS_BASIC_CALLBACK+=from/PASS
TODO_CODECS_BASIC_CALLBACK+=from/UTF-16BE
TODO_CODECS_BASIC_CALLBACK+=from/UTF-16LE
TODO_CODECS_BASIC_CALLBACK+=from/UTF-32BE
TODO_CODECS_BASIC_CALLBACK+=from/UTF-32LE
TODO_CODECS_BASIC_CALLBACK+=from/_GB18030
TODO_CODECS_BASIC_CALLBACK+=from/_UTF-8
TODO_CODECS_BASIC_CALLBACK+=inter/AMBIGUOUS-PAD
TODO_CODECS_BASIC_CALLBACK+=inter/AMBIGUOUS-UNPAD
TODO_CODECS_BASIC_CALLBACK+=inter/BIG5-DEFRAG
TODO_CODECS_BASIC_CALLBACK+=inter/FROM_ALIAS
TODO_CODECS_BASIC_CALLBACK+=inter/SCORE
TODO_CODECS_BASIC_CALLBACK+=inter/SCORE_TRAIN
TODO_CODECS_BASIC_CALLBACK+=inter/SPLIT
TODO_CODECS_BASIC_CALLBACK+=inter/TO_ALIAS
TODO_CODECS_BASIC_CALLBACK+=inter/TRIM-WIDTH
TODO_CODECS_BASIC_CALLBACK+=inter/WHITESPACE-DERAIL
TODO_CODECS_BASIC_CALLBACK+=inter/WHITESPACE-RERAIL
TODO_CODECS_BASIC_CALLBACK+=inter/WIDTH
TODO_CODECS_BASIC_CALLBACK+=inter/ZH-STRINGS
TODO_CODECS_BASIC_CALLBACK+=to/ANY
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HTML-CNS11643-IMG
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HTML-INFO
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HTML-UNICODE-IMG
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV_LOG
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV_STDOUT
TODO_CODECS_BASIC_CALLBACK+=to/ESCAPE
TODO_CODECS_BASIC_CALLBACK+=to/NULL
TODO_CODECS_BASIC_CALLBACK+=to/PASS
TODO_CODECS_BASIC_CALLBACK+=to/RAW
TODO_CODECS_BASIC_CALLBACK+=to/UCS-2BE
TODO_CODECS_BASIC_CALLBACK+=to/UCS-2LE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-16BE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-16LE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-32BE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-32LE
TODO_CODECS_BASIC_CALLBACK+=to/_GB18030
TODO_CODECS_BASIC_CALLBACK+=to/_UTF-8

TODO_CODECS_CHINESE_TABLE=
TODO_CODECS_CHINESE_TABLE+=from/CCCII
TODO_CODECS_CHINESE_TABLE+=from/_CNS11643
TODO_CODECS_CHINESE_TABLE+=from/_CP936
TODO_CODECS_CHINESE_TABLE+=from/_CP950
TODO_CODECS_CHINESE_TABLE+=from/_GB2312
TODO_CODECS_CHINESE_TABLE+=from/_UAO241
TODO_CODECS_CHINESE_TABLE+=inter/CHEWING
TODO_CODECS_CHINESE_TABLE+=inter/CNS11643
TODO_CODECS_CHINESE_TABLE+=inter/HAN_PINYIN
TODO_CODECS_CHINESE_TABLE+=inter/UNICODE
TODO_CODECS_CHINESE_TABLE+=inter/ZH_COMP
TODO_CODECS_CHINESE_TABLE+=inter/ZH_DECOMP
TODO_CODECS_CHINESE_TABLE+=to/CCCII
TODO_CODECS_CHINESE_TABLE+=to/_CNS11643
TODO_CODECS_CHINESE_TABLE+=to/_GB2312
TODO_CODECS_CHINESE_TABLE+=to/_UAO241

TODO_CODECS_CHINESE_CALLBACK=
TODO_CODECS_CHINESE_CALLBACK+=from/_CNS11643
TODO_CODECS_CHINESE_CALLBACK+=to/_CNS11643

TODO_CODECS_EBCDIC_TABLE=
TODO_CODECS_EBCDIC_TABLE+=from/IBM-37
TODO_CODECS_EBCDIC_TABLE+=from/IBM-930
TODO_CODECS_EBCDIC_TABLE+=from/IBM-933
TODO_CODECS_EBCDIC_TABLE+=from/IBM-935
TODO_CODECS_EBCDIC_TABLE+=from/IBM-937
TODO_CODECS_EBCDIC_TABLE+=from/IBM-939
TODO_CODECS_EBCDIC_TABLE+=to/IBM-37
TODO_CODECS_EBCDIC_TABLE+=to/IBM-930
TODO_CODECS_EBCDIC_TABLE+=to/IBM-933
TODO_CODECS_EBCDIC_TABLE+=to/IBM-935
TODO_CODECS_EBCDIC_TABLE+=to/IBM-937
TODO_CODECS_EBCDIC_TABLE+=to/IBM-939

TODO_CODECS_EBCDIC_CALLBACK=
TODO_CODECS_EBCDIC_CALLBACK+=from/IBM-930
TODO_CODECS_EBCDIC_CALLBACK+=from/IBM-933
TODO_CODECS_EBCDIC_CALLBACK+=from/IBM-935
TODO_CODECS_EBCDIC_CALLBACK+=from/IBM-937
TODO_CODECS_EBCDIC_CALLBACK+=from/IBM-939
TODO_CODECS_EBCDIC_CALLBACK+=to/IBM-930
TODO_CODECS_EBCDIC_CALLBACK+=to/IBM-933
TODO_CODECS_EBCDIC_CALLBACK+=to/IBM-935
TODO_CODECS_EBCDIC_CALLBACK+=to/IBM-937
TODO_CODECS_EBCDIC_CALLBACK+=to/IBM-939

all: libbsdconv bsdconv_mktable meta bsdconv_completion bsdconv codecs

alias:
	python tools/mkalias.py codecs/from/alias codecs/inter/FROM_ALIAS.txt
	@printf "014C,014F,0143,0141,014C,0145\t?\n" >> codecs/inter/FROM_ALIAS.txt
	python tools/mkalias.py codecs/inter/alias codecs/inter/INTER_ALIAS.txt
	python tools/mkalias.py codecs/to/alias codecs/inter/TO_ALIAS.txt
	@printf "014C,014F,0143,0141,014C,0145\t?\n" >> codecs/inter/TO_ALIAS.txt

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/share/bsdconv/from
	mkdir -p build/share/bsdconv/inter
	mkdir -p build/share/bsdconv/to

installdir:
	mkdir -p ${PREFIX}/bin
	mkdir -p ${PREFIX}/lib
	mkdir -p ${PREFIX}/include
	mkdir -p ${PREFIX}/share/bsdconv/from
	mkdir -p ${PREFIX}/share/bsdconv/inter
	mkdir -p ${PREFIX}/share/bsdconv/to

libbsdconv: builddir src/libbsdconv.c src/bsdconv.h
	$(CC) ${CFLAGS} src/libbsdconv.c -fPIC -shared -o build/lib/${SHLIBNAME} ${LIBS}

bsdconv: builddir libbsdconv meta src/bsdconv.h src/bsdconv.c
	$(CC) ${CFLAGS} src/bsdconv.c -L./build/lib/ -o build/bin/bsdconv -lbsdconv ${LIBS}

bsdconv_mktable: builddir src/bsdconv.h src/bsdconv_mktable.c
	$(CC) ${CFLAGS} -DUSE_FMALLOC src/libfmalloc.c src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

bsdconv_completion: builddir libbsdconv src/bsdconv.h src/bsdconv_completion.c
	$(CC) ${CFLAGS} src/bsdconv_completion.c -L./build/lib -o build/bin/bsdconv_completion -lbsdconv ${LIBS}

bsdconv_dbg: builddir libbsdconv src/libbsdconv.c src/bsdconv.h src/bsdconv_dbg.c
	$(CC) ${CFLAGS} src/libbsdconv.c src/bsdconv_dbg.c -o build/bin/bsdconv_dbg ${LIBS}

codecs_basic_table: builddir bsdconv_mktable
	for item in ${TODO_CODECS_BASIC_TABLE} ; do \
		./build/bin/bsdconv_mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
	done

codecs_basic_callback: builddir libbsdconv
	for item in ${TODO_CODECS_BASIC_CALLBACK} ; do \
		$(CC) ${CFLAGS} codecs/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; \
	done

codecs_chinese_table: builddir bsdconv_mktable
	for item in ${TODO_CODECS_CHINESE_TABLE} ; do \
		./build/bin/bsdconv_mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
	done

codecs_chinese_callback: builddir libbsdconv
	for item in ${TODO_CODECS_CHINESE_CALLBACK} ; do \
		$(CC) ${CFLAGS} codecs/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; \
	done

codecs_ebcdic_table: builddir bsdconv_mktable
	for item in ${TODO_CODECS_EBCDIC_TABLE} ; do \
		./build/bin/bsdconv_mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
	done

codecs_ebcdic_callback: builddir libbsdconv
	for item in ${TODO_CODECS_EBCDIC_CALLBACK} ; do \
		$(CC) ${CFLAGS} codecs/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; \
	done

codecs: codecs_basic codecs_chinese codecs_ebcdic
codecs_basic: codecs_basic_table codecs_basic_callback
codecs_chinese: codecs_chinese_table codecs_chinese_callback
codecs_ebcdic: codecs_ebcdic_table codecs_ebcdic_callback

meta: libbsdconv
	if [ ${SHLIBNAME} != libbsdconv.so ]; then \
		ln -sf libbsdconv.so.${SHLIBVER} build/lib/libbsdconv.so ; \
	fi
	cp src/bsdconv.h build/include
	cp codecs/from/alias build/share/bsdconv/from/alias
	cp codecs/inter/alias build/share/bsdconv/inter/alias
	cp codecs/to/alias build/share/bsdconv/to/alias

clean:
	rm -rf build

install: installdir install_main install_basic install_chinese install_ebcdic

install_main:
	install -m 555 build/bin/bsdconv ${PREFIX}/bin
	install -m 555 build/bin/bsdconv_mktable ${PREFIX}/bin
	install -m 555 build/bin/bsdconv_completion ${PREFIX}/bin
	install -m 444 build/include/bsdconv.h ${PREFIX}/include
	install -m 444 build/lib/${SHLIBNAME} ${PREFIX}/lib
	install -m 444 build/share/bsdconv/from/alias ${PREFIX}/share/bsdconv/from/alias
	install -m 444 build/share/bsdconv/inter/alias ${PREFIX}/share/bsdconv/inter/alias
	install -m 444 build/share/bsdconv/to/alias ${PREFIX}/share/bsdconv/to/alias
	if [ ${SHLIBNAME} != libbsdconv.so ]; then \
		ln -sf libbsdconv.so.${SHLIBVER} ${PREFIX}/lib/libbsdconv.so ; \
	fi

install_basic:
	for item in ${TODO_CODECS_BASIC_TABLE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
	done
	for item in ${TODO_CODECS_BASIC_CALLBACK} ; do \
		install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; \
	done

install_chinese:
	for item in ${TODO_CODECS_CHINESE_TABLE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
	done
	for item in ${TODO_CODECS_CHINESE_CALLBACK} ; do \
		install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; \
	done

install_ebcdic:
	for item in ${TODO_CODECS_EBCDIC_TABLE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
	done
	for item in ${TODO_CODECS_EBCDIC_CALLBACK} ; do \
		install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; \
	done

plist:
	@echo bin/bsdconv
	@echo bin/bsdconv_completion
	@echo bin/bsdconv_mktable
	@echo include/bsdconv.h
	@echo lib/libbsdconv.so
	@echo lib/${SHLIBNAME}
	@echo %%DATADIR%%/from/alias
	@echo %%DATADIR%%/inter/alias
	@echo %%DATADIR%%/to/alias
	@for item in ${TODO_CODECS_BASIC_TABLE} ; do \
		echo %%DATADIR%%/$${item} ; \
	done
	@for item in ${TODO_CODECS_BASIC_CALLBACK} ; do \
		echo %%DATADIR%%/$${item}.so ; \
	done
	@for item in ${TODO_CODECS_CHINESE_TABLE} ; do \
		echo %%CHINESE%%%%DATADIR%%/$${item} ; \
	done
	@for item in ${TODO_CODECS_CHINESE_CALLBACK} ; do \
		echo %%CHINESE%%%%DATADIR%%/$${item}.so ; \
	done
	@for item in ${TODO_CODECS_EBCDIC_TABLE} ; do \
		echo %%EBCDIC%%%%DATADIR%%/$${item} ; \
	done
	@for item in ${TODO_CODECS_EBCDIC_CALLBACK} ; do \
		echo %%EBCDIC%%%%DATADIR%%/$${item}.so ; \
	done
	@echo @dirrmtry %%DATADIR%%/to
	@echo @dirrmtry %%DATADIR%%/inter
	@echo @dirrmtry %%DATADIR%%/from
	@echo @dirrmtry %%DATADIR%%

URL=	http://cnmc.tw/~buganini/chvar/engine.php?action=dump
chvar:
	wget -O codecs/inter/ZHTW.txt "${URL}&mode=norml&for=tw"
	wget -O codecs/inter/ZHCN.txt "${URL}&mode=norml&for=cn"
	wget -O codecs/inter/ZH_FUZZY_TW.txt "${URL}&mode=fuzzy&for=tw"
	wget -O codecs/inter/ZH_FUZZY_CN.txt "${URL}&mode=fuzzy&for=cn"
	@for file in ZHTW ZHCN ZH_FUZZY_TW ZH_FUZZY_CN; do \
		sed -i '' -e 's|^|01|g' "codecs/inter/$${file}.txt" ; \
		sed -i '' -e 's|	|	01|g' "codecs/inter/$${file}.txt" ; \
	done
	wget -O codecs/to/CP950_TRANS.txt "${URL}&mode=trans&for=cp950"
	wget -O codecs/to/CP936_TRANS.txt "${URL}&mode=trans&for=cp936"
	@for file in CP950_TRANS CP936_TRANS ; do \
		sed -i '' -e 's|^|01|g' "codecs/to/$${file}.txt" ; \
	done

