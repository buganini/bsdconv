/*
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
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

int main(int argc, char *argv[]){
	char *t;
	char *tmp=NULL;
	struct bsdconv_instance *ins;
	FILE *inf, *otf;
	char *in, *to;

	if(argc<2){
		fprintf(stderr, "Usage:\n\t %s from:[inter:..]to [input|- [output|-|=]]\nfrom,inter,to in form of codec[,codec2..]\n", argv[0]);
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
		to=argv[3];
		if(strcmp(to,"=")==0){
			to=argv[2];
		}
		if(strcmp(to,"-")==0){
			otf=stdout;
		}else{
			tmp=malloc(strlen(to)+8);
			strcpy(tmp, to);
			strcat(tmp, ".XXXXXX");
			if(mktemp(tmp)==NULL){
				free(tmp);
				fprintf(stderr, "Failed creating temp file.\n");
				exit(1);
			}
			otf=fopen(tmp,"w");
			if(!otf){
				fprintf(stderr, "Unable to open output file %s\n", to);
				exit(1);
			}
		}
	}else{
		to=NULL;
		otf=stdout;
	}

	ins=bsdconv_create(argv[1]);
	if(!ins){
		t=bsdconv_error();
		printf("%s\n", t);
		free(t);
		exit(1);
	}
	bsdconv_init(ins);
	do{
		in=malloc(IBUFLEN);
		ins->input.data=in;
		ins->input.len=fread(in, 1, IBUFLEN, inf);
		ins->input.flags|=F_FREE;
		if(ins->input.len==0){
			ins->flush=1;
		}
		ins->output_mode=BSDCONV_FILE;
		ins->output.data=otf;
		bsdconv(ins);
	}while(ins->flush==0);

	if(inf!=stdin){
		fclose(inf);
	}
	if(to){
		fclose(otf);
		unlink(to);
		rename(tmp,to);
		free(tmp);
	}

	fprintf(stderr, "Decoding failure: %u\n", ins->ierr);
	fprintf(stderr, "Encoding failure: %u\n", ins->oerr);
	bsdconv_destroy(ins);

	return 0;
}

