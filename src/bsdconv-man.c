#include <stdio.h>
#include <string.h>
#include "bsdconv.h"

static void usage(char *a0){
	fprintf(stderr, "Usage:\n\t%s phase_type codec\n\t%s phase_type/codec\n", a0, a0);
	exit(1);
}

static int man(char *pc){
	char *phase=NULL;
	char *codec=NULL;
	char *codec_filename=NULL;
	char *path;
	char *phase_dir=NULL;
	int iphase=0;
	char buf[BUFSIZ];
	size_t len;
	FILE *fp;

	codec=phase=strdup(pc);
	strsep(&codec, "/");
	if(codec==NULL){
		free(phase);
		return 1;
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
	}else if(strcmp(phase, "FILTER")==0){
		iphase=FILTER;
		phase_dir="filter";
	}else{
		free(phase);
		free(codec);
		return 1;
	}

	path=getCodecDir();
	chdir(path);
	chdir(phase_dir);

	codec_filename=malloc(strlen(codec)+5);
	sprintf(codec_filename, "%s.man", codec);
	fp=fopen(codec_filename, "rb");
	if(fp){
		if(fscanf(fp, ".redirect %s", buf)){
			fclose(fp);
			free(codec_filename);
			free(phase);
			free(path);
			return man(buf);
		}
		(void) fseek(fp, 0L, SEEK_SET);
		printf("%s/%s:\n", phase, codec);
		do{
			len=fread(buf, 1, sizeof(buf), fp);
			fwrite(buf, len, 1, stdout);
		}while(!feof(fp));
		fclose(fp);
		free(codec_filename);
		free(phase);
		free(path);
		return 0;
	}
	free(codec_filename);

	char *a=bsdconv_solve_alias(iphase, codec);
	if(a!=NULL){
		printf("%s/%s:\n", phase, codec);
		printf("Alias to %s\n", a);
	}else if(bsdconv_module_check(iphase, codec)){
		printf("No man page for such codec\n");
	}else{
		printf("No such codec\n");
	}
	free(a);

	free(phase);
	free(path);
	return 0;
}

int main(int argc, char *argv[]){
	char *pc;
	int r;
	if(argc==2){
		r=man(argv[1]);
	}else if(argc==3){
		pc=malloc(strlen(argv[1])+strlen(argv[2])+2);
		sprintf(pc, "%s/%s", argv[1], argv[2]);
		r=man(pc);
		free(pc);
	}else{
		usage(argv[0]);
		return 1;
	}

	if(r)
		usage(argv[0]);

	return r;
}
