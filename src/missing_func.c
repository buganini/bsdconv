#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
char * strsep(char **stringp, const char *delim){
	char *r=*stringp;
	if(!**stringp) return NULL;
	for(;**stringp && !index(delim, **stringp);++(*stringp));
	**stringp=0x0;
	(*stringp)++;
	return r;
}

char * index(const char *s, int c){
	for(;*s && *s!=c;++s);
	if(*s) return (char *)s;
	return NULL;
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

char * getwd(char *buf){
	char b[512], *r;
	int l;
	getcwd(b,512);
	l=strlen(b);
	r=malloc(l);
	memcpy(r,b,l);
	return r;
}
#endif

