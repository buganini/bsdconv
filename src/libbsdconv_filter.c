/*
 * Copyright (c) 2009-2014 Kuan-Chung Chiu <buganini@gmail.com>
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
