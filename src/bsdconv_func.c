/*
 * Copyright (c) 2009,2010 Kuan-Chung Chiu <buganini@gmail.com>
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

#ifdef WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif
#include <string.h>
#include "bsdconv.h"

int loadcodec(struct bsdconv_codec_t *cd, char *path, int maponly){
#ifdef WIN32
	if ((cd->fd=CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))==INVALID_HANDLE_VALUE){
		SetLastError(EOPNOTSUPP);
		return 0;
	}
	cd->md=CreateFileMapping(cd->fd, NULL, PAGE_READONLY, 0,0, NULL);
	if(!cd->md){
		CloseHandle(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
	cd->data_z=cd->z=MapViewOfFile(cd->md, FILE_MAP_READ, 0,0,0);
	if(!cd->z){
		CloseHandle(cd->md);
		CloseHandle(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
#else
	struct stat stat;
	if((cd->fd=open(path, O_RDONLY))==-1){
		SetLastError(EOPNOTSUPP);
		return 0;
	}
	fstat(cd->fd, &stat);
	cd->maplen=stat.st_size;
	if((cd->data_z=cd->z=mmap(0,stat.st_size,PROT_READ, MAP_PRIVATE,cd->fd,0))==MAP_FAILED){
		close(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
#endif

	cd->dl=NULL;
	cd->cbcreate=NULL;
	cd->cbinit=NULL;
	cd->callback=NULL;
	cd->cbdestroy=NULL;
	if(maponly) return 1;
	strcat(path, "." SHLIBEXT);

#ifdef WIN32
	if((cd->dl=LoadLibrary(path))){
		cd->callback=GetProcAddress(cd->dl,"callback");
		cd->cbcreate=GetProcAddress(cd->dl,"cbcreate");
		cd->cbinit=GetProcAddress(cd->dl,"cbinit");
		cd->cbdestroy=GetProcAddress(cd->dl,"cbdestroy");
	}
#else
	if((cd->dl=dlopen(path, RTLD_LAZY))){
		cd->callback=dlsym(cd->dl,"callback");
		cd->cbcreate=dlsym(cd->dl,"cbcreate");
		cd->cbinit=dlsym(cd->dl,"cbinit");
		cd->cbdestroy=dlsym(cd->dl,"cbdestroy");
	}
#endif
	return 1;
}

void unloadcodec(struct bsdconv_codec_t *cd){
#ifdef WIN32
	if(cd->dl){
		FreeLibrary(cd->dl);
	}
	UnmapViewOfFile(cd->z);
	CloseHandle(cd->md);
	CloseHandle(cd->fd);
#else
	if(cd->dl){
		dlclose(cd->dl);
	}
	munmap(cd->z, cd->maplen);
	close(cd->fd);
#endif
}

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
	if(*s) return s;
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

char *getwd(char *buf){
	char b[512], *r;
	int l;
	getcwd(b,512);
	l=strlen(b);
	r=malloc(l);
	memcpy(r,b,l);
	return r;
}
#endif
