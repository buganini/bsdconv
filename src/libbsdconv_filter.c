struct bsdconv_filter *load_filter(const char *_name){
	struct bsdconv_filter *filter;

	char *cwd;
	char *c;
	char path[PATH_MAX+1];
	char *name=strdup(_name);
	strtoupper(name);

	while(!bsdconv_module_check(FILTER, name)){
		c=bsdconv_solve_alias(FILTER, name);
		if(c==NULL || strcmp(c, name)==0){
			free(name);
			free(c);
			return NULL;
		}
		free(name);
		name=c;
	}
	cwd=getcwd(NULL, 0);
	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}
	chdir(MODULES_SUBPATH);
	chdir("filter");
	REALPATH(name, path);
	chdir(cwd);
	free(cwd);
	free(name);
	strcat(path, "." SHLIBEXT);

	filter=malloc(sizeof(struct bsdconv_filter));
	filter->so=OPEN_SHAREOBJECT(path);
	if(!filter->so){
		free(filter);
		return NULL;
	}

	filter->cbfilter=SHAREOBJECT_SYMBOL(filter->so, "cbfilter");

	return filter;
}

void unload_filter(struct bsdconv_filter *filter){
	CLOSE_SHAREOBJECT(filter->so);
	free(filter);
}
