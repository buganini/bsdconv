CFLAGS=-Wall

All: libbsdconv src/bsdconv_mktable bsdconv codecs

libbsdconv:
	$(CC) src/libbsdconv.c -shared -o src/libbsdconv.so

bsdconv:
	$(CC) src/libbsdconv.so src/bsdconv.c -o src/bsdconv

clean:
	rm src/bsdconv src/libbsdconv.so src/bsdconv_mktable

codecs:
