CFLAGS=-Wall -g

All: builddir libbsdconv bsdconv_mktable bsdconv codecs

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/share/bsdconv

libbsdconv:
	$(CC) ${CFLAGS} src/libbsdconv.c -shared -o build/lib/libbsdconv.so

bsdconv:
	$(CC) ${CFLAGS} src//libbsdconv.c src/bsdconv.c -o build/bin/bsdconv

bsdconv_mktable:
	$(CC) ${CFLAGS} src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

clean:
	rm -rf build

codecs:
