#ifdef USE_FMALLOC

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "fmalloc.h"

const char *fmalloc_template="/tmp/.fmalloc.XXXXXX";
struct fmalloc_entry * fmalloc_pools=NULL;
int fmalloc_num=0;

void * fmalloc(size_t s){
	void *m;
	char *tmpfile;
	int tmpfd;
	size_t o_offset;
	struct fmalloc_entry * last;
	if(fmalloc_pools==NULL || ((fmalloc_pools->offset+s) > FMALLOC_SIZE)){
		if(fmalloc_num < FMALLOC_NUM){
			tmpfile=strdup(fmalloc_template);
			if((tmpfd=mkstemp(tmpfile))==-1){
				free(tmpfile);
				return malloc(s);
			}
			unlink(tmpfile);
			free(tmpfile);
			ftruncate(tmpfd, FMALLOC_SIZE);
			m=mmap(0, FMALLOC_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, tmpfd, 0);
			if(m==MAP_FAILED){
				return malloc(s);
			}
			last=fmalloc_pools;
			fmalloc_pools=malloc(sizeof(struct fmalloc_entry));
			fmalloc_pools->z=m;
			fmalloc_pools->offset=0;
			fmalloc_pools->fd=tmpfd;
			fmalloc_pools->next=last;
			fmalloc_num+=1;
		}else{
			return malloc(s);
		}
	}
	o_offset=fmalloc_pools->offset;
	fmalloc_pools->offset+=s;
	return fmalloc_pools->z + o_offset;
}

void fmfree(void *p){
	struct fmalloc_entry *entry=fmalloc_pools;
	while(entry){
		if(p>=entry->z && p<entry->z+entry->offset){
			return;
		}
		entry=entry->next;
	}
	free(p);
}

void fmsync(void){
	struct fmalloc_entry *entry=fmalloc_pools;
	while(entry){
		msync(entry->z, entry->offset, MS_SYNC);
		entry=entry->next;
	}
}

void fmcleanup(void){
	struct fmalloc_entry *next=fmalloc_pools;
	while(fmalloc_pools){
		next=fmalloc_pools->next;
		munmap(fmalloc_pools->z, FMALLOC_SIZE);
		close(fmalloc_pools->fd);
		free(fmalloc_pools);
		fmalloc_pools=next;
	}
}
#endif
