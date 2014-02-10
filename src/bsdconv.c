#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <fcntl.h>
#endif
#include "bsdconv.h"

#define IBUFLEN 1024

#define LIST_MODULES(type) do{ \
	list=bsdconv_modules_list(type); \
	p=list; \
	while(*p!=NULL){ \
		printf("\t%s\n", *p); \
		bsdconv_free(*p); \
		p+=1; \
	} \
	bsdconv_free(list); \
}while(0);

void list_modules(){
	char **list;
	char **p;

	printf("[From]\n");
	LIST_MODULES(FROM);
	printf("[Inter]\n");
	LIST_MODULES(INTER);
	printf("[To]\n");
	LIST_MODULES(TO);
	printf("[Filter]\n");
	LIST_MODULES(FILTER);
	printf("[Scorer]\n");
	LIST_MODULES(SCORER);

	exit(0);
}

void bsdconv_file(struct bsdconv_instance *ins, FILE *in, FILE *out, const char *filename){
	char *ib;
	bsdconv_init(ins);
	do{
		ib=bsdconv_malloc(IBUFLEN);
		ins->input.data=ib;
		ins->input.flags|=F_FREE;
		ins->input.next=NULL;
		if((ins->input.len=fread(ib, 1, IBUFLEN, in))==0){
			ins->flush=1;
		}
		ins->output_mode=BSDCONV_FILE;
		ins->output.data=out;
		bsdconv(ins);
	}while(ins->flush==0);

	struct bsdconv_counter_entry *c=ins->counter;
	char f=0;
	while(c!=NULL){
		if(c->val){
			if(!f){
				f=1;
				if(filename)
					fprintf(stderr, "\nFile: %s\n", filename);
			}
			fprintf(stderr, "%s: %zu\n", c->key, c->val);
		}
		c=c->next;
	}
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
		fprintf(stderr, "Usage:\n\t %s conversion|-l [-i] [file] [...]\n\t\t-i:\tsave in-place\n\t\t-l:\tlist codecs\n", argv[0]);
		exit(1);
	}
	i=2;

	if(strcmp(argv[1],"-l")==0){
		list_modules();
		return 0;
	}

	if(argc>2) while(i<argc){
		if(strcmp(argv[i],"-i")==0)
			inplace=1;
		else if(strcmp(argv[i],"-l")==0)
			list_modules();
		else
			break;
		i+=1;
	}


#ifdef WIN32
	setmode(STDIN_FILENO, O_BINARY);
	setmode(STDOUT_FILENO, O_BINARY);
#endif

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
			inf=fopen(argv[i],"rb");
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
			otf=fdopen(fd, "wb");
			if(!otf){
				fprintf(stderr, "Unable to open output file for %s\n", argv[i]);
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
			inf=fopen(argv[i],"rb");
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
