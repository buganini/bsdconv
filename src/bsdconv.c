#include <stdio.h>
#include "bsdconv.h"

int main(int argc, char *argv[]){
	struct bsdconv_instruction ins;
	struct bsdconv_t *cd;
	FILE *inf=fopen(argv[4],"rb");
	unsigned char in[1024], out[1024];
	cd=bsdconv_create(argv[1], argv[2], argv[3]);
	bsdconv_init(cd, &ins, in, 1024, out, 1024);
	int r;
	do{
		if(ins.feed_len) ins.feed_len=fread(ins.feed, 1, ins.feed_len, inf);
		r=bsd_conv(cd, &ins);
		if(ins.back_len)fwrite(ins.back, 1, ins.back_len, stdout);
	}while(r);
	bsdconv_destroy(cd);
	return 0;
}

