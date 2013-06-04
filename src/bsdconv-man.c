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
	const char *conv=NULL;
	struct bsdconv_instance *ins;
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
		conv="ASCII:FROM_ALIAS:ASCII";
		phase_dir="from";
		iphase=FROM;
	}else if(strcmp(phase, "INTER")==0){
		conv="ASCII:INTER_ALIAS:ASCII";
		phase_dir="inter";
		iphase=INTER;
	}else if(strcmp(phase, "TO")==0){
		conv="ASCII:TO_ALIAS:ASCII";
		phase_dir="to";
		iphase=TO;
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

	ins=bsdconv_create(conv);
	bsdconv_init(ins);
	ins->input.data=codec;
	ins->input.len=strlen(codec);
	ins->flush=1;
	bsdconv(ins);
	ins->output_mode=BSDCONV_AUTOMALLOC;
	ins->output.len=1;
	bsdconv(ins);
	UCP(ins->output.data)[ins->output.len]=0;
	if(strcmp(ins->output.data, codec)!=0){
		printf("%s/%s:\n", phase, codec);
		printf("Alias to %s\n", CP(ins->output.data));
	}else if(bsdconv_codec_check(iphase, codec)){
		printf("No man page for such codec\n");
	}else{
		printf("No such codec\n");
	}
	free(ins->output.data);
	bsdconv_destroy(ins);

	free(phase);
	free(codec);
	free(path);
	return 0;
}
