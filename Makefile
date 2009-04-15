CFLAGS=-Wall

All: builddir libbsdconv bsdconv_mktable bsdconv codecs

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/share/bsdconv

libbsdconv:
	$(CC) src/libbsdconv.c -shared -o build/lib/libbsdconv.so

bsdconv:
	$(CC) build/lib/libbsdconv.so src/bsdconv.c -o build/bin/bsdconv

bsdconv_mktable:
	$(CC) src/bsdconv_mktable.c -o build/bin/bsdconv_mktable

clean:
	rm -rf build

codecs:
