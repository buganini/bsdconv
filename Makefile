PREFIX?=/usr/local
BSDCONV_PATH?=${PREFIX}
CFLAGS+=-Wall -DPREFIX='"${PREFIX}"' -DBSDCONV_PATH='"${BSDCONV_PATH}"'
SHLIBVER=6

TODO_CODECS_BASIC_TABLE=
TODO_CODECS_BASIC_TABLE+=from/3F
TODO_CODECS_BASIC_TABLE+=from/ANSI-CONTROL
TODO_CODECS_BASIC_TABLE+=from/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=from/ASCII-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=from/ASCII
TODO_CODECS_BASIC_TABLE+=from/BIG5-5C
TODO_CODECS_BASIC_TABLE+=from/BSDCONV
TODO_CODECS_BASIC_TABLE+=from/BSDCONV_KEYWORD
TODO_CODECS_BASIC_TABLE+=from/BSDCONV_LOG
TODO_CODECS_BASIC_TABLE+=from/BYTE
TODO_CODECS_BASIC_TABLE+=from/_CP1251
TODO_CODECS_BASIC_TABLE+=from/_CP1252
TODO_CODECS_BASIC_TABLE+=from/_CP874
TODO_CODECS_BASIC_TABLE+=from/_CP949
TODO_CODECS_BASIC_TABLE+=from/ESCAPE
TODO_CODECS_BASIC_TABLE+=from/_GBK
TODO_CODECS_BASIC_TABLE+=from/_ISO-8859-1
TODO_CODECS_BASIC_TABLE+=from/NUL
TODO_CODECS_BASIC_TABLE+=from/PASS
TODO_CODECS_BASIC_TABLE+=from/_SHIFT-JIS
TODO_CODECS_BASIC_TABLE+=from/SKIP
TODO_CODECS_BASIC_TABLE+=from/_UAO250
TODO_CODECS_BASIC_TABLE+=from/UTF-16BE
TODO_CODECS_BASIC_TABLE+=from/UTF-16LE
TODO_CODECS_BASIC_TABLE+=from/UTF-32BE
TODO_CODECS_BASIC_TABLE+=from/UTF-32LE
TODO_CODECS_BASIC_TABLE+=from/_UTF-8
TODO_CODECS_BASIC_TABLE+=inter/AMBIGUOUS-PAD
TODO_CODECS_BASIC_TABLE+=inter/AMBIGUOUS-UNPAD
TODO_CODECS_BASIC_TABLE+=inter/BIG5-DEFRAG
TODO_CODECS_BASIC_TABLE+=inter/FROM_ALIAS
TODO_CODECS_BASIC_TABLE+=inter/FULL
TODO_CODECS_BASIC_TABLE+=inter/HALF
TODO_CODECS_BASIC_TABLE+=inter/INTER_ALIAS
TODO_CODECS_BASIC_TABLE+=inter/JP_PINYIN
TODO_CODECS_BASIC_TABLE+=inter/LOWER
TODO_CODECS_BASIC_TABLE+=inter/MAC
TODO_CODECS_BASIC_TABLE+=inter/NL2BR
TODO_CODECS_BASIC_TABLE+=inter/SCORE
TODO_CODECS_BASIC_TABLE+=inter/SCORE_TRAIN
TODO_CODECS_BASIC_TABLE+=inter/SPLIT
TODO_CODECS_BASIC_TABLE+=inter/TO_ALIAS
TODO_CODECS_BASIC_TABLE+=inter/UNIX
TODO_CODECS_BASIC_TABLE+=inter/UPPER
TODO_CODECS_BASIC_TABLE+=inter/UPSIDEDOWN
TODO_CODECS_BASIC_TABLE+=inter/WHITESPACE-DERAIL
TODO_CODECS_BASIC_TABLE+=inter/WHITESPACE-RERAIL
TODO_CODECS_BASIC_TABLE+=inter/WIDTH
TODO_CODECS_BASIC_TABLE+=inter/WIN
TODO_CODECS_BASIC_TABLE+=inter/ZHCN
TODO_CODECS_BASIC_TABLE+=inter/ZHTW
TODO_CODECS_BASIC_TABLE+=inter/ZHTW_WORDS
TODO_CODECS_BASIC_TABLE+=to/3F
TODO_CODECS_BASIC_TABLE+=to/ANSI-CONTROL
TODO_CODECS_BASIC_TABLE+=to/ASCII-ESCAPED-UNICODE
TODO_CODECS_BASIC_TABLE+=to/ASCII-HEX-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=to/ASCII-HTML-CNS11643-IMG
TODO_CODECS_BASIC_TABLE+=to/ASCII-HTML-INFO
TODO_CODECS_BASIC_TABLE+=to/ASCII-HTML-UNICODE-IMG
TODO_CODECS_BASIC_TABLE+=to/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC_TABLE+=to/ASCII
TODO_CODECS_BASIC_TABLE+=to/BIG5-5C
TODO_CODECS_BASIC_TABLE+=to/BSDCONV
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_KEYWORD
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_LOG
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_RAW
TODO_CODECS_BASIC_TABLE+=to/BSDCONV_STDOUT
TODO_CODECS_BASIC_TABLE+=to/BYTE
TODO_CODECS_BASIC_TABLE+=to/_CP936
TODO_CODECS_BASIC_TABLE+=to/CP936_TRANS
TODO_CODECS_BASIC_TABLE+=to/_CP950
TODO_CODECS_BASIC_TABLE+=to/CP950_TRANS
TODO_CODECS_BASIC_TABLE+=to/_GBK
TODO_CODECS_BASIC_TABLE+=to/_ISO-8859-1
TODO_CODECS_BASIC_TABLE+=to/NUL
TODO_CODECS_BASIC_TABLE+=to/NULL
TODO_CODECS_BASIC_TABLE+=to/PASS
TODO_CODECS_BASIC_TABLE+=to/RAW
TODO_CODECS_BASIC_TABLE+=to/_UAO250
TODO_CODECS_BASIC_TABLE+=to/UCS-2BE
TODO_CODECS_BASIC_TABLE+=to/UCS-2LE
TODO_CODECS_BASIC_TABLE+=to/UNICODE
TODO_CODECS_BASIC_TABLE+=to/UTF-16BE
TODO_CODECS_BASIC_TABLE+=to/UTF-16LE
TODO_CODECS_BASIC_TABLE+=to/UTF-32BE
TODO_CODECS_BASIC_TABLE+=to/UTF-32LE
TODO_CODECS_BASIC_TABLE+=to/_UTF-8

TODO_CODECS_EXTRA_TABLE=
TODO_CODECS_EXTRA_TABLE+=from/_CNS11643
TODO_CODECS_EXTRA_TABLE+=from/_CP936
TODO_CODECS_EXTRA_TABLE+=from/_CP950
TODO_CODECS_EXTRA_TABLE+=from/_GB2312
TODO_CODECS_EXTRA_TABLE+=from/_UAO241
TODO_CODECS_EXTRA_TABLE+=inter/CHEWING
TODO_CODECS_EXTRA_TABLE+=inter/CNS11643
TODO_CODECS_EXTRA_TABLE+=inter/HAN_PINYIN
TODO_CODECS_EXTRA_TABLE+=inter/UNICODE
TODO_CODECS_EXTRA_TABLE+=inter/ZH_COMP
TODO_CODECS_EXTRA_TABLE+=inter/ZH_DECOMP
TODO_CODECS_EXTRA_TABLE+=to/_CNS11643
TODO_CODECS_EXTRA_TABLE+=to/_GB2312
TODO_CODECS_EXTRA_TABLE+=to/_UAO241

TODO_CODECS_BASIC_CALLBACK=
TODO_CODECS_BASIC_CALLBACK+=from/ANSI-CONTROL
TODO_CODECS_BASIC_CALLBACK+=from/ASCII-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC_CALLBACK+=from/BSDCONV
TODO_CODECS_BASIC_CALLBACK+=from/BSDCONV_LOG
TODO_CODECS_BASIC_CALLBACK+=from/ESCAPE
TODO_CODECS_BASIC_CALLBACK+=from/PASS
TODO_CODECS_BASIC_CALLBACK+=from/SKIP
TODO_CODECS_BASIC_CALLBACK+=from/UTF-16BE
TODO_CODECS_BASIC_CALLBACK+=from/UTF-16LE
TODO_CODECS_BASIC_CALLBACK+=from/UTF-32BE
TODO_CODECS_BASIC_CALLBACK+=from/UTF-32LE
TODO_CODECS_BASIC_CALLBACK+=from/_UTF-8
TODO_CODECS_BASIC_CALLBACK+=inter/AMBIGUOUS-PAD
TODO_CODECS_BASIC_CALLBACK+=inter/AMBIGUOUS-UNPAD
TODO_CODECS_BASIC_CALLBACK+=inter/BIG5-DEFRAG
TODO_CODECS_BASIC_CALLBACK+=inter/SCORE
TODO_CODECS_BASIC_CALLBACK+=inter/SCORE_TRAIN
TODO_CODECS_BASIC_CALLBACK+=inter/SPLIT
TODO_CODECS_BASIC_CALLBACK+=inter/WHITESPACE-DERAIL
TODO_CODECS_BASIC_CALLBACK+=inter/WHITESPACE-RERAIL
TODO_CODECS_BASIC_CALLBACK+=inter/WIDTH
TODO_CODECS_BASIC_CALLBACK+=to/3F
TODO_CODECS_BASIC_CALLBACK+=to/ANSI-CONTROL
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-ESCAPED-UNICODE
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HEX-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HTML-CNS11643-IMG
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HTML-INFO
TODO_CODECS_BASIC_CALLBACK+=to/ASCII-HTML-UNICODE-IMG
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV_LOG
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV_RAW
TODO_CODECS_BASIC_CALLBACK+=to/BSDCONV_STDOUT
TODO_CODECS_BASIC_CALLBACK+=to/NULL
TODO_CODECS_BASIC_CALLBACK+=to/PASS
TODO_CODECS_BASIC_CALLBACK+=to/RAW
TODO_CODECS_BASIC_CALLBACK+=to/UCS-2BE
TODO_CODECS_BASIC_CALLBACK+=to/UCS-2LE
TODO_CODECS_BASIC_CALLBACK+=to/UNICODE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-16BE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-16LE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-32BE
TODO_CODECS_BASIC_CALLBACK+=to/UTF-32LE
TODO_CODECS_BASIC_CALLBACK+=to/_UTF-8

TODO_CODECS_EXTRA_CALLBACK=
TODO_CODECS_EXTRA_CALLBACK+=from/_CNS11643
TODO_CODECS_EXTRA_CALLBACK+=to/_CNS11643

all: libbsdconv bsdconv_mktable meta bsdconv codecs

alias:
	python tools/mkalias.py codecs/from/alias codecs/inter/FROM_ALIAS.txt
	python tools/mkalias.py codecs/inter/alias codecs/inter/INTER_ALIAS.txt
	python tools/mkalias.py codecs/to/alias codecs/inter/TO_ALIAS.txt

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
	$(CC) ${CFLAGS} src/libbsdconv.c -fPIC -shared -o build/lib/libbsdconv.so.${SHLIBVER}

bsdconv: builddir libbsdconv meta src/bsdconv.h src/bsdconv.c
	$(CC) ${CFLAGS} src/bsdconv.c -L./build/lib/ -lbsdconv -o build/bin/bsdconv

bsdconv_mktable: builddir src/bsdconv.h
	$(CC) ${CFLAGS} -DUSE_FMALLOC src/libfmalloc.c src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

codecs_basic_table: builddir bsdconv_mktable
	for item in ${TODO_CODECS_BASIC_TABLE} ; do \
		./build/bin/bsdconv_mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
	done

codecs_basic_callback: builddir libbsdconv
	for item in ${TODO_CODECS_BASIC_CALLBACK} ; do \
		$(CC) ${CFLAGS} -L./build/lib/ -lbsdconv -fPIC -shared -o ./build/share/bsdconv/$${item}.so codecs/$${item}.c ; \
	done

codecs_extra_table: builddir bsdconv_mktable
	for item in ${TODO_CODECS_EXTRA_TABLE} ; do \
		./build/bin/bsdconv_mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
	done

codecs_extra_callback: builddir libbsdconv
	for item in ${TODO_CODECS_EXTRA_CALLBACK} ; do \
		$(CC) ${CFLAGS} -L./build/lib/ -lbsdconv -fPIC -shared -o ./build/share/bsdconv/$${item}.so codecs/$${item}.c ; \
	done

codecs: codecs_basic codecs_extra
codecs_basic: codecs_basic_table codecs_basic_callback
codecs_extra: codecs_extra_table codecs_extra_callback

meta: libbsdconv
	ln -sf libbsdconv.so.${SHLIBVER} build/lib/libbsdconv.so
	cp src/bsdconv.h build/include
	cp codecs/from/alias build/share/bsdconv/from/alias
	cp codecs/inter/alias build/share/bsdconv/inter/alias
	cp codecs/to/alias build/share/bsdconv/to/alias

clean:
	rm -rf build

install: installdir install_main install_basic install_extra

install_main:
	install -m 555 build/bin/bsdconv ${PREFIX}/bin
	install -m 555 build/bin/bsdconv_mktable ${PREFIX}/bin
	install -m 444 build/include/bsdconv.h ${PREFIX}/include
	install -m 444 build/lib/libbsdconv.so.${SHLIBVER} ${PREFIX}/lib
	install -m 444 build/share/bsdconv/from/alias ${PREFIX}/share/bsdconv/from/alias
	install -m 444 build/share/bsdconv/inter/alias ${PREFIX}/share/bsdconv/inter/alias
	install -m 444 build/share/bsdconv/to/alias ${PREFIX}/share/bsdconv/to/alias
	ln -sf libbsdconv.so.${SHLIBVER} ${PREFIX}/lib/libbsdconv.so

install_basic:
	for item in ${TODO_CODECS_BASIC_TABLE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
	done
	for item in ${TODO_CODECS_BASIC_CALLBACK} ; do \
		install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; \
	done

install_extra:
	for item in ${TODO_CODECS_EXTRA_TABLE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
	done
	for item in ${TODO_CODECS_EXTRA_CALLBACK} ; do \
		install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; \
	done

plist:
	@echo bin/bsdconv
	@echo bin/bsdconv_mktable
	@echo include/bsdconv.h
	@echo lib/libbsdconv.so
	@echo lib/libbsdconv.so.${SHLIBVER}
	@echo %%DATADIR%%/from/alias
	@echo %%DATADIR%%/inter/alias
	@echo %%DATADIR%%/to/alias
	@for item in ${TODO_CODECS_BASIC_TABLE} ; do \
		echo %%DATADIR%%/$${item} ; \
	done
	@for item in ${TODO_CODECS_BASIC_CALLBACK} ; do \
		echo %%DATADIR%%/$${item}.so ; \
	done
	@for item in ${TODO_CODECS_EXTRA_TABLE} ; do \
		echo %%EXTRA%%%%DATADIR%%/$${item} ; \
	done
	@for item in ${TODO_CODECS_EXTRA_CALLBACK} ; do \
		echo %%EXTRA%%%%DATADIR%%/$${item}.so ; \
	done
	@echo @dirrmtry %%DATADIR%%/to
	@echo @dirrmtry %%DATADIR%%/inter
	@echo @dirrmtry %%DATADIR%%/from
	@echo @dirrmtry %%DATADIR%%
