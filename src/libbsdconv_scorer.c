struct bsdconv_scorer *load_scorer(const char *_name){
	struct bsdconv_scorer *scorer;

	char *cwd;
	char *c;
	char path[PATH_MAX+1];
	char *name=strdup(_name);
	strtoupper(name);

	while(!bsdconv_module_check(SCORER, name)){
		c=bsdconv_solve_alias(SCORER, name);
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
	chdir("scorer");
	REALPATH(name, path);
	chdir(cwd);
	free(cwd);
	free(name);
	strcat(path, "." SHLIBEXT);

	scorer=malloc(sizeof(struct bsdconv_scorer));
	scorer->so=OPEN_SHAREOBJECT(path);
	if(!scorer->so){
		free(scorer);
		return NULL;
	}

	scorer->cbscorer=SHAREOBJECT_SYMBOL(scorer->so, "cbscorer");

	return scorer;
}

void unload_scorer(struct bsdconv_scorer *scorer){
	CLOSE_SHAREOBJECT(scorer->so);
	free(scorer);
}
