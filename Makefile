PREFIX?=/usr/local
CFLAGS=-Wall -g -DPREFIX='"${PREFIX}"'

All: builddir libbsdconv bsdconv_mktable bsdconv codecs

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/share/bsdconv/from
	mkdir -p build/share/bsdconv/inter
	mkdir -p build/share/bsdconv/to

libbsdconv:
	$(CC) ${CFLAGS} src/libbsdconv.c -shared -o build/lib/libbsdconv.so

bsdconv:
	$(CC) ${CFLAGS} src/libbsdconv.c src/bsdconv.c -o build/bin/bsdconv

bsdconv_mktable:
	$(CC) ${CFLAGS} src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

codecs: bsdconv_mktable
	cd codecs && \
	find */*.txt -type f | awk -F. '{cmd="../build/bin/bsdconv_mktable "$$1"."$$2" ../build/share/bsdconv/"$$1; system(cmd);}' && \
	find */*.c -type f | awk -F. '{cmd="gcc -shared -o ../build/share/bsdconv/"$$1".so "$$1"."$$2; system(cmd);}'

clean:
	rm -rf build

install:
	cp -R build/ ${PREFIX}
