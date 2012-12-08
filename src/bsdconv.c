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
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "bsdconv.h"

#define IBUFLEN 1024

#define LIST_CODECS(type) do{ \
	list=bsdconv_codecs_list(type); \
	p=list; \
	while(*p!=NULL){ \
		printf("\t%s\n", *p); \
		bsdconv_free(*p); \
		p+=1; \
	} \
	bsdconv_free(list); \
}while(0);

void list_codecs(){
	char **list;
	char **p;
	
	printf("[From]\n");
	LIST_CODECS(FROM);
	printf("[Inter]\n");
	LIST_CODECS(INTER);
	printf("[To]\n");
	LIST_CODECS(TO);

	exit(0);
}

void bsdconv_file(struct bsdconv_instance *ins, FILE *in, FILE *out, const char *filename){
	char *ib;
	bsdconv_init(ins);
	do{
		ib=bsdconv_malloc(IBUFLEN);
		ins->input.data=ib;
		ins->input.flags|=F_FREE;
		if((ins->input.len=fread(ib, 1, IBUFLEN, in))==0){
			ins->flush=1;
		}
		ins->output_mode=BSDCONV_FILE;
		ins->output.data=out;
		bsdconv(ins);
	}while(ins->flush==0);

	if(filename && (ins->ierr || ins->oerr || ins->full || ins->half || ins->ambi || ins->score))
		fprintf(stderr, "\nFile: %s\n", filename);
	if(ins->ierr || ins->oerr){
		fprintf(stderr, "Decoding failure: %u\n", ins->ierr);
		fprintf(stderr, "Encoding failure: %u\n", ins->oerr);
	}
	if(ins->full || ins->half || ins->ambi){
		fprintf(stderr, "Full width: %u\n", ins->full);
		fprintf(stderr, "Half width: %u\n", ins->half);
		fprintf(stderr, "Ambi width: %u\n", ins->ambi);
	}
	if(ins->score)
		fprintf(stderr, "Score: %lf\n", ins->score);
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
		fprintf(stderr, "Usage:\n\t %s conversion [-il] [file] [...]\n\t\t-i:\tsave in-place\n\t\t-l:\tlist codecs\n", argv[0]);
		exit(1);
	}
	i=2;

	if(strcmp(argv[1],"-l")==0)
		list_codecs();

	if(argc>2) while(i<argc){
		if(strcmp(argv[i],"-i")==0)
			inplace=1;
		else if(strcmp(argv[i],"-l")==0)
			list_codecs();
		else
			break;
		i+=1;
	}

	ins=bsdconv_create(argv[1]);
	if(!ins){
		t=bsdconv_error();
		fprintf(stderr, "%s\n", t);
		free(t);
		exit(1);
	}

	if(i>=argc){
		bsdconv_file(ins, stdin, stdout, NULL);
	}else for(;i<argc;++i){
		if(inplace){
			inf=fopen(argv[i],"r");
			if(inf==NULL){
				fprintf(stderr, "Failed opening file %s.\n", argv[i]);
				bsdconv_destroy(ins);
				exit(1);
			}
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
#ifndef WIN32
			struct stat stat;
			fstat(fileno(inf), &stat);
			fchown(fileno(otf), stat.st_uid, stat.st_gid);
			fchmod(fileno(otf), stat.st_mode);
#endif
			bsdconv_file(ins, inf, otf, argv[i]);
			fclose(inf);
			fclose(otf);
			unlink(argv[i]);
			rename(tmp,argv[i]);
			free(tmp);

		}else{
			inf=fopen(argv[i],"r");
			if(inf==NULL){
				fprintf(stderr, "Failed opening file %s.\n", argv[i]);
				bsdconv_destroy(ins);
				exit(1);
			}
			bsdconv_file(ins, inf, stdout, argv[i]);
			fclose(inf);
		}
	}
	bsdconv_destroy(ins);

	return 0;
}

