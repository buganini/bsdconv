/*
 * Copyright (c) 2009 Kuan-Chung Chiu <buganini@gmail.com>
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

#ifndef BSDCONV_H
#define BSDCONV_H

#include <unistd.h>
#ifdef WIN32
#include <windows.h>
#endif

enum bsdconv_phase_type {
	INPUT,
	FROM,
	INTER,
	TO
};

enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,
	SUBMATCH,
	SUBROUTINE,
	NEXTPHASE,
	DUMMY,
};

enum bsdconv_output_mode{
	BSDCONV_HOLD,
	BSDCONV_AUTOMALLOC,
	BSDCONV_PREMALLOCED,
	BSDCONV_FILE,
	BSDCONV_FD,
};

struct data_st{
	char *data;
	size_t len;
	struct data_st *next;
};

struct data_rt{
	void *data;
	size_t len;
	struct data_rt *next;
	char setmefree;
};

struct state_st{
	char status;
	struct data_st *data;
	struct state_st *sub[257];
};

struct state_rt{
	char status;
	struct data_rt *data;
	struct state_st *sub[257];
};

struct bsdconv_instance{
	int output_mode;
	
	struct data_rt input, output;

	char flush;

	struct bsdconv_phase *phase;
	int phasen, phase_index;
	unsigned int ierr, oerr;

	struct data_rt *pool;
};

struct bsdconv_phase{
	struct data_rt *bak, *match, *data_head, *data_tail, *data;
	struct state_rt state;
	int index;
	int i;
	char pend;
	char type;
	struct bsdconv_codec_t *codec;
	int codecn;
};

struct bsdconv_codec_t {
#ifdef WIN32
	HANDLE fd;
	HANDLE md;
	HMODULE dl;
#else
	int fd;
	size_t maplen;
	void *dl;
#endif
	char *z;
	char *data_z;
	char *desc;
	void (*callback)(struct bsdconv_instance *);
	void *(*cbcreate)(void);
	void (*cbinit)(struct bsdconv_codec_t *, void *);
	void (*cbdestroy)(void *);
	void *priv;
};

#define PATH_BUF_SIZE 512

#ifndef EDOOFUS
#define EDOOFUS 88
#endif

#ifdef WIN32
#define EOPNOTSUPP ERROR_NOT_SUPPORTED
#define ENOMEM ERROR_NOT_ENOUGH_MEMORY
#define EINVAL ERROR_BAD_COMMAND
#define SHLIBEXT "dll"
#define REALPATH(buf, path) GetFullPathName(buf, PATH_BUF_SIZE, path, NULL)
char * strsep(char **, const char *);
char * index(const char *, int);
char * getwd(char *);
#else
#define SetLastError(n) errno=n
#define GetLastError() errno
#define SHLIBEXT "so"
#define REALPATH(buf, path) realpath(buf, path)
#endif

#define LISTCPY(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	DATA_MALLOC((X)->next);	\
	(X)=(X)->next;	\
	memcpy((X), (char *)((Z)+(uintptr_t)data_ptr), sizeof(struct data_st));	\
	data_ptr=(X)->next;	\
	(X)->next=NULL;	\
	(X)->data=((Z)+(uintptr_t)(X)->data);	\
	(X)->setmefree=0; \
}

#define LISTFREE(X,Y,Z)	while((X)->next!=(Y)){	\
	data_ptr=(X)->next->next;	\
	if((X)->next->setmefree)	\
		free((X)->next->data);	\
	if((Z)==(X)->next){	\
		(Z)=(X);	\
	}	\
	DATA_FREE((X)->next);	\
	(X)->next=data_ptr;	\
}

#define CP(X) ((char *)(X)) 
#define UCP(X) ((unsigned char *)(X)) 

#define DATA_MALLOC(X) do{if(ins->pool){(X)=ins->pool; ins->pool=ins->pool->next;}else{(X)=malloc(sizeof(struct data_rt));}}while(0)
#define DATA_FREE(X) do{ if((X)->setmefree) free((X)->data); (X)->next=ins->pool; ins->pool=(X);}while(0)

struct bsdconv_instance *bsdconv_create(const char *);
void bsdconv_init(struct bsdconv_instance *);
void bsdconv_destroy(struct bsdconv_instance *);
void bsdconv(struct bsdconv_instance *);
char * bsdconv_error(void);

int loadcodec(struct bsdconv_codec_t *, char *, int);
void unloadcodec(struct bsdconv_codec_t *);

#endif
