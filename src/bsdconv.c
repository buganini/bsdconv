/*
 * Copyright (c) 2009 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsdconv.h"

#define IBUFLEN 1024
#define OBUFLEN 1024

int main(int argc, char *argv[]){
	char *t;
	struct bsdconv_instance *ins;
	FILE *inf, *otf;
	unsigned char in[IBUFLEN], out[OBUFLEN];

	int r;
	if(argc<2){
		fprintf(stderr, "Usage:\n\t %s from:[inter:..]to [input|- [output|-]]\nfrom,inter,to in form of codec[,codec2..]\n", argv[0]);
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

	ins=bsdconv_create(argv[1]);
	if(!ins){
		t=bsdconv_error();
		printf("%s\n", t);
		free(t);
		exit(1);
	}
	ins->in_buf=in;
	ins->in_len=IBUFLEN;
	ins->out_buf=out;
	ins->out_len=OBUFLEN;
	ins->mode=BSDCONV_BB;
	bsdconv_init(ins);
	do{
		if(ins->feed_len) ins->feed_len=fread(ins->feed, 1, ins->feed_len, inf);
		r=bsdconv(ins);
		if(ins->back_len)fwrite(ins->back, 1, ins->back_len, otf);
	}while(r);

	fprintf(stderr, "Decoding failure: %u\n", ins->ierr);
	fprintf(stderr, "Encoding failure: %u\n", ins->oerr);

	bsdconv_destroy(ins);
	if(inf!=stdin){
		fclose(inf);
	}
	if(otf!=stdout){
		fclose(otf);
	}
	return 0;
}

