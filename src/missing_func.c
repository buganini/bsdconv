#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
char * strsep(char **stringp, const char *delim){
	char *r=*stringp;
	if(!**stringp) return NULL;
	for(;**stringp && !strchr(delim, **stringp);++(*stringp));
	**stringp=0x0;
	(*stringp)++;
	return r;
}

char * strndup(const char *str, size_t len){
	char *r;
	size_t l=strlen(str);
	if(len<l)l=len;
	r=malloc(l+1);
	memcpy(r,str,l);
	r[l]=0;
	return r;
}

int mkstemp(char *tmpl){
	int ret;
	mktemp(tmpl);
	ret=open(tmpl,O_RDWR|O_BINARY|O_CREAT|O_EXCL);
	return ret;
}

char *index(char *s, int c){
	while(*s){
		if(*s==c)
			return s;
		s+=1;
	}
	return NULL;
}

#endif

