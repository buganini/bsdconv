/*
 * Copyright (c) 2013 Kuan-Chung Chiu <buganini@gmail.com>
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

#define _BSDCONV_INTERNAL

#include <stdio.h>
#include <string.h>
#include "bsdconv.h"

static inline void strtoupper(char *s){
	char *c;
	for(c=s;*c;++c){
		if(*c>='a' && *c<='z'){
			*c=*c-'a'+'A';
		}
	}
}

void usage(char *a0){
	fprintf(stderr, "Usage:\n\t%s phase_type codec\n\t%s phase_type/codec\n", a0, a0);
	exit(1);
}

int main(int argc, char *argv[]){
	char *phase=NULL;
	char *codec=NULL;
	char *codec_filename=NULL;
	char *path;
	char *phase_dir=NULL;
	int iphase=0;
	char buf[BUFSIZ];
	size_t len;
	FILE *fp;

	if(argc==2){
		codec=phase=strdup(argv[1]);
		strsep(&codec, "/");
		if(codec==NULL){
			usage(argv[0]);
		}
		codec=strdup(codec);
	}else if(argc==3){
		phase=strdup(argv[1]);
		codec=strdup(argv[2]);
	}else{
		usage(argv[0]);
	}
	strtoupper(phase);
	strtoupper(codec);

	if(strcmp(phase, "FROM")==0){
		iphase=FROM;
		phase_dir="from";
	}else if(strcmp(phase, "INTER")==0){
		iphase=INTER;
		phase_dir="inter";
	}else if(strcmp(phase, "TO")==0){
		iphase=TO;
		phase_dir="to";
	}else{
		free(phase);
		free(codec);
		usage(argv[0]);
	}

	path=getCodecDir();
	chdir(path);
	chdir(phase_dir);

	codec_filename=malloc(strlen(codec)+5);
	sprintf(codec_filename, "%s.man", codec);
	fp=fopen(codec_filename, "rb");
	if(fp){
		printf("%s/%s:\n", phase, codec);
		do{
			len=fread(buf, 1, sizeof(buf), fp);
			fwrite(buf, len, 1, stdout);
		}while(!feof(fp));
		fclose(fp);
		free(codec_filename);
		free(phase);
		free(codec);
		free(path);
		return 0;
	}
	free(codec_filename);

	char *a=bsdconv_solve_alias(iphase, codec);
	if(a!=NULL){
		printf("%s/%s:\n", phase, codec);
		printf("Alias to %s\n", a);
		free(a);
	}else if(bsdconv_codec_check(iphase, codec)){
		printf("No man page for such codec\n");
	}else{
		printf("No such codec\n");
	}

	free(phase);
	free(codec);
	free(path);
	return 0;
}
