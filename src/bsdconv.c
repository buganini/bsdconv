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

void bsdconv_file(struct bsdconv_instance *ins, FILE *in, FILE *out){
	char *ib;
	bsdconv_init(ins);
	do{
		ib=malloc(IBUFLEN);
		ins->input.data=ib;
		ins->input.flags|=F_FREE;
		if((ins->input.len=fread(ib, 1, IBUFLEN, in))==0){
			ins->flush=1;
		}
		ins->output_mode=BSDCONV_FILE;
		ins->output.data=out;
		bsdconv(ins);
	}while(ins->flush==0);

	fprintf(stderr, "Decoding failure: %u\n", ins->ierr);
	fprintf(stderr, "Encoding failure: %u\n", ins->oerr);
	if(ins->full || ins->half || ins->ambi){
		fprintf(stderr, "Full width: %u\n", ins->full);
		fprintf(stderr, "Half width: %u\n", ins->half);
		fprintf(stderr, "Ambi width: %u\n", ins->ambi);
	}
	if(ins->score)
		fprintf(stderr, "Score: %u\n", ins->score);
}

int main(int argc, char *argv[]){
	char *t;
	int fd;
	char *tmp=NULL;
	struct bsdconv_instance *ins;
	FILE *inf=NULL, *otf=stdout;
	int inplace=0;
	int i;

	if(argc<2){
		fprintf(stderr, "Usage:\n\t %s conversion [-i] [file] [...]\n", argv[0]);
		exit(1);
	}
	i=2;
	if(argc>2 && strcmp(argv[i],"-i")==0){
		i+=1;
		inplace=1;
	}

	ins=bsdconv_create(argv[1]);
	if(!ins){
		t=bsdconv_error();
		fprintf(stderr, "%s\n", t);
		free(t);
		exit(1);
	}

	if(i>=argc){
		bsdconv_file(ins, stdin, stdout);
	}else for(;i<argc;++i){
		if(inplace){
			tmp=malloc(strlen(argv[i])+8);
			strcpy(tmp, argv[i]);
			strcat(tmp, ".XXXXXX");
			if((fd=mkstemp(tmp))==-1){
				free(tmp);
				fprintf(stderr, "Failed creating temp file.\n");
				bsdconv_destroy(ins);
				exit(1);
			}
			otf=fdopen(fd,"w");
			if(!otf){
				fprintf(stderr, "Unable to open output file %s\n", argv[i]);
				bsdconv_destroy(ins);
				exit(1);
			}
			inf=fopen(argv[i],"r");
			bsdconv_file(ins, inf, otf);
			fclose(inf);
			fclose(otf);
			unlink(argv[i]);
			rename(tmp,argv[i]);
			free(tmp);

		}else{
			inf=fopen(argv[i],"r");
			bsdconv_file(ins, inf, stdout);
			fclose(inf);
		}
	}
	bsdconv_destroy(ins);

	return 0;
}

