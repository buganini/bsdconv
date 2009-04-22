#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsdconv.h"

int main(int argc, char *argv[]){
	struct bsdconv_instruction ins;
	struct bsdconv_t *cd;
	FILE *inf, *otf;
	unsigned char in[1024], out[1024];

	int r;
	if(argc<2){
		fprintf(stderr, "Usage:\n\t %s from[,from2...]:[inter[,inter2,...]]:to[,to2...] [input|- [output|-]]\n", argv[0]);
		exit(1);
	}
	if(argc>2){
		if(strcmp(argv[2],"-")==0){
			inf=stdin;
		}else{
			inf=fopen(argv[2],"r");
			if(!inf){
				fprintf(stderr, "Unable to open input file %s\n", argv[2]);
				exit(1);
			}
		}
	}else{
		inf=stdin;
	}
	if(argc>3){
		if(strcmp(argv[3],"-")==0){
			otf=stdout;
		}else{
			otf=fopen(argv[3],"w");
			if(!otf){
				fprintf(stderr, "Unable to open output file %s\n", argv[3]);
				exit(1);
			}
		}
	}else{
		otf=stdout;
	}

	cd=bsdconv_create(argv[1]);
	bsdconv_init(cd, &ins, in, 1024, out, 1024);
	do{
		if(ins.feed_len) ins.feed_len=fread(ins.feed, 1, ins.feed_len, inf);
		r=bsd_conv(cd, &ins);
		if(ins.back_len)fwrite(ins.back, 1, ins.back_len, otf);
	}while(r);

	fprintf(stderr, "Decoding failure: %u\n", ins.ierr);
	fprintf(stderr, "Encoding failure: %u\n", ins.oerr);

	bsdconv_destroy(cd);
	if(inf!=stdin){
		fclose(inf);
	}
	if(otf!=stdout){
		fclose(otf);
	}
	return 0;
}

