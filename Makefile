PREFIX?=/usr/local
CFLAGS+=-Wall -DPREFIX='"${PREFIX}"'
SHLIBVER=2

all: builddir libbsdconv bsdconv_mktable bsdconv codecs meta

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/share/bsdconv/from
	mkdir -p build/share/bsdconv/inter
	mkdir -p build/share/bsdconv/to

libbsdconv:
	$(CC) ${CFLAGS} src/libbsdconv.c -fPIC -shared -o build/lib/libbsdconv.so.${SHLIBVER}

bsdconv:
	$(CC) ${CFLAGS} src/libbsdconv.c src/bsdconv.c -o build/bin/bsdconv

bsdconv_mktable:
	$(CC) ${CFLAGS} src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

codecs: bsdconv_mktable
	cd codecs && \
	find */*.txt -type f | awk -F. '{cmd="../build/bin/bsdconv_mktable "$$1"."$$2" ../build/share/bsdconv/"$$1; system(cmd);}' && \
	find */*.c -type f | awk -F. '{cmd="gcc ${CFLAGS} -fPIC -shared -o ../build/share/bsdconv/"$$1".so "$$1"."$$2; system(cmd);}'

meta:
	ln -sf libbsdconv.so.${SHLIBVER} build/lib/libbsdconv.so
	cat codecs/aliases.map | awk '{cmd="ln -sf ../"$$2" build/share/bsdconv/"$$1; system(cmd);}'
	cp src/bsdconv.h build/include

clean:
	rm -rf build

install:
	cp -R build/ ${PREFIX}
