#ifndef FMALLOC_H
#define FMALLOC_H

#ifdef USE_FMALLOC

void * fmalloc(size_t s);
void fmfree(void *p);
void fmsync(void);
void fmcleanup(void);

struct fmalloc_entry {
	void *z;
	size_t offset;
	int fd;
	struct fmalloc_entry *next;
};

#define FMALLOC_SIZE 256*1024*1024
#define FMALLOC_NUM 6
#define FMALLOC(X) fmalloc(X)
#define FFREE(X) fmfree(X)

#else

#define FMALLOC(X) malloc(X)
#define FFREE(X) free(X)

#endif

#endif
