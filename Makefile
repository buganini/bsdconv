PREFIX?=/usr/local
CFLAGS+=-Wall -DPREFIX='"${PREFIX}"'
SHLIBVER=3

all: builddir libbsdconv bsdconv_mktable bsdconv codecs meta

alias:
.for t in from inter to
	cd codecs/$t/ && \
	cp alias alias.tmp && \
	find *.txt |awk 'BEGIN{FS=".txt"}; {print $$1"\t"$$1}' >> alias.tmp
	python tools/mkalias.py codecs/$t/alias.tmp codecs/inter/${t:U}_ALIAS.txt
	rm codecs/$t/alias.tmp
.endfor

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/share/bsdconv/from
	mkdir -p build/share/bsdconv/inter
	mkdir -p build/share/bsdconv/to

libbsdconv:
	$(CC) ${CFLAGS} src/bsdconv_func.c src/libbsdconv.c -fPIC -shared -o build/lib/libbsdconv.so.${SHLIBVER}

bsdconv:
	$(CC) ${CFLAGS} src/bsdconv_func.c src/libbsdconv.c src/bsdconv.c -o build/bin/bsdconv

bsdconv_mktable:
	$(CC) ${CFLAGS} src/bsdconv_func.c src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

codecs_table: bsdconv_mktable
	cd codecs && \
	find */*.txt -type f | awk -F. '{cmd="../build/bin/bsdconv_mktable "$$1"."$$2" ../build/share/bsdconv/"$$1; system(cmd);}'

codecs_callback:
	cd codecs && \
	find */*.c -type f | awk -F. '{cmd="$(CC) ${CFLAGS} -fPIC -shared -o ../build/share/bsdconv/"$$1".so ../src/bsdconv_func.c "$$1"."$$2; system(cmd);}'

codecs: codecs_table codecs_callback

meta:
	ln -sf libbsdconv.so.${SHLIBVER} build/lib/libbsdconv.so
	cp src/bsdconv.h build/include

clean:
	rm -rf build

install:
	cp -R build/ ${PREFIX}
