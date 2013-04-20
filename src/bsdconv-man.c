#include <stdio.h>
#include <string.h>
#include "bsdconv.h"

inline void strtoupper(char *s){
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
	char *phase;
	char *codec;
	char *codec_filename;
	char *path;
	char *phase_dir;
	char buf[BUFSIZ];
	int phase_i;
	size_t len;
	FILE *fp, *pp;

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
		phase_i=FROM;
		phase_dir="from";
	}else if(strcmp(phase, "INTER")==0){
		phase_i=INTER;
		phase_dir="inter";
	}else if(strcmp(phase, "TO")==0){
		phase_i=TO;
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

	printf("blah\n");

	free(phase);
	free(codec);
	free(path);
	return 0;
}
