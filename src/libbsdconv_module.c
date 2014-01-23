char * bsdconv_solve_alias(int type, char *_codec){
	char *ret;
	char *codec;
	struct bsdconv_instance *ins;
	bsdconv_counter_t *ct;
	switch(type){
		case FROM:
			ins=bsdconv_create("ASCII:ALIAS-FROM,COUNT#ERR:ASCII");
			break;
		case INTER:
			ins=bsdconv_create("ASCII:ALIAS-INTER,COUNT#ERR:ASCII");
			break;
		case TO:
			ins=bsdconv_create("ASCII:ALIAS-TO,COUNT#ERR:ASCII");
			break;
		case FILTER:
			ins=bsdconv_create("ASCII:ALIAS-FILTER,COUNT#ERR:ASCII");
			break;
		default:
			return NULL;
	}
	if(ins==NULL){
		return NULL;
	}
	codec=strdup(_codec);
	strtoupper(codec);
	bsdconv_init(ins);
	ins->output_mode=BSDCONV_AUTOMALLOC;
	ins->output.len=1;
	ins->input.data=codec;
	ins->input.len=strlen(codec);
	ins->input.flags=F_FREE;
	ins->input.next=NULL;
	ins->flush=1;
	bsdconv(ins);
	ret=ins->output.data;
	ret[ins->output.len]=0;
	ct=bsdconv_counter(ins, "ERR");
	if(*ct>0){
		free(ret);
		ret=NULL;
	}
	bsdconv_destroy(ins);
	return ret;
}

int bsdconv_module_check(int type, const char *_codec){
	int ret=0;
	char *cwd;
	char *codec=NULL;
	FILE *fp;
	char *c;

	cwd=getcwd(NULL, 0);

	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}

	chdir(MODULES_SUBPATH);
	switch(type){
		case FROM:
			chdir("from");
			codec=strdup(_codec);
			strtoupper(codec);
			break;
		case INTER:
			chdir("inter");
			codec=strdup(_codec);
			strtoupper(codec);
			break;
		case TO:
			chdir("to");
			codec=strdup(_codec);
			strtoupper(codec);
			break;
		case FILTER:
			chdir("filter");
			codec=malloc(strlen(_codec) + strlen("." SHLIBEXT) + 1);
			strcpy(codec, _codec);
			strtoupper(codec);
			strcat(codec, "." SHLIBEXT);
			break;
	}

	fp=fopen(codec, "rb");
	if(fp!=NULL){
		fclose(fp);
		ret=1;
	}
	free(codec);

	chdir(cwd);
	free(cwd);

	return ret;
}

int scmp(const void *a, const void *b){
    return strcmp(*(char **)a, *(char **)b);
}

char ** bsdconv_modules_list(int phase_type){
	char **list=malloc(sizeof(char *) * 8);
	int size=8;
	int length=0;
	char *cwd;
	char *c;
	DIR *dir;
	struct dirent *d;
	FILE *fp;
	char buf[256];
	const char *type;
	cwd=getcwd(NULL, 0);

	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}
	list[0]=NULL;
	chdir(MODULES_SUBPATH);
	switch(phase_type){
		case FROM:
			type="from";
			break;
		case INTER:
			type="inter";
			break;
		case TO:
			type="to";
			break;
		case FILTER:
			type="filter";
			break;
		default:
			return list;
	}
	dir=opendir(type);
	if(dir!=NULL){
		if(phase_type==FILTER){
			while((d=readdir(dir))!=NULL){
				if(strstr(d->d_name, ".so")==NULL)
					continue;
				if(length>=size){
					size+=8;
					list=realloc(list, sizeof(char *) * size);
				}
				list[length]=strdup(d->d_name);
				c=list[length];
				strsep(&c, ".");
				length+=1;
			}
		}else{
			while((d=readdir(dir))!=NULL){
				if(strstr(d->d_name, ".")!=NULL || strcmp(d->d_name, "alias")==0)
					continue;
				if(length>=size){
					size+=8;
					list=realloc(list, sizeof(char *) * size);
				}
				list[length]=strdup(d->d_name);
				length+=1;
			}
		}
		closedir(dir);
	}
	chdir(type);
	fp=fopen("alias","rb");
	if(fp!=NULL){
		while(fgets(buf, sizeof(buf), fp)!=NULL){
			if(buf[0]=='#')
				continue;
			if(length>=size){
				size+=8;
				list=realloc(list, sizeof(char *) * size);
			}
			c=buf;
			list[length]=strdup(strsep(&c, "\t"));
			length+=1;
		}
		fclose(fp);
	}
	if(length>=size){
		size+=8;
		list=realloc(list, sizeof(char *) * size);
	}
	qsort(list, length, sizeof(char *), scmp);
	list[length]=NULL;
	chdir(cwd);
	free(cwd);
	return list;
}
