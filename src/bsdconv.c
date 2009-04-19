#include <stdio.h>
#include <stdlib.h>
#include "bsdconv.h"

int main(int argc, char *argv[]){
	struct bsdconv_instruction ins;
	struct bsdconv_t *cd;
	FILE *inf=fopen(argv[2],"rb");
	unsigned char in[1024], out[1024];
	cd=bsdconv_create(argv[1]);
	bsdconv_init(cd, &ins, in, 1024, out, 1024);
	int r;
	if(!inf){
		fprintf(stderr, "Unable to open file %s\n", argv[2]);
		exit(1);
	}
	do{
		if(ins.feed_len) ins.feed_len=fread(ins.feed, 1, ins.feed_len, inf);
		r=bsd_conv(cd, &ins);
		if(ins.back_len)fwrite(ins.back, 1, ins.back_len, stdout);
	}while(r);
	bsdconv_destroy(cd);
	return 0;
}

