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

enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,
	SUBMATCH,
	SUBROUTINE,
	NEXTPHASE,
	DUMMY,
};

enum bsdconv_mode{
	BSDCONV_BB,
	BSDCONV_BC,
	BSDCONV_CB,
	BSDCONV_CC,
	BSDCONV_BM,
	BSDCONV_CM,
};

struct data_s{
	char *data;
	size_t len;
	struct data_s *next;
};

struct state_s{
	char status;
	struct data_s *data;
	struct state_s *sub[257];
};

struct bsdconv_instance{
	int mode;
	char *in_buf;
	size_t in_len;
	char *out_buf;
	size_t out_len;
	char *feed;
	size_t feed_len;
	char *back;
	size_t back_len;
	char *from_data;

	struct bsdconv_phase *phase;
	int phasen, phase_index;
	unsigned int ierr, oerr;
	char *from_bak;
};

struct bsdconv_phase{
	struct data_s *bak, *match, *data_head, *data_tail, *data;
	struct state_s state;
	int index;
	char pend;
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

#ifdef WIN32
#define EOPNOTSUPP ERROR_NOT_SUPPORTED
#define ENOMEM ERROR_NOT_ENOUGH_MEMORY
#define EINVAL ERROR_BAD_COMMAND
#define SHLIBEXT "dll"
#define REALPATH(buf, path) GetFullPathName(buf, PATH_BUF_SIZE, path, NULL)
char * strsep(char **, const char *);
char * index(const char *, int);
#else
#define SetLastError(n) errno=n
#define GetLastError() errno
#define SHLIBEXT "so"
#define REALPATH(buf, path) realpath(buf, path)
#endif

#define LISTCPY(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	(X)->next=malloc(sizeof(struct data_s));	\
	(X)=(X)->next;	\
	memcpy((X), (char *)((Z)+(uintptr_t)data_ptr), sizeof(struct data_s));	\
	data_ptr=(X)->next;	\
	(X)->next=NULL;	\
	ptr=(char *)((Z)+(uintptr_t)(X)->data);	\
	(X)->data=malloc((X)->len + 1);	\
	memcpy((X)->data, ptr, (X)->len);	\
}

#define LISTFREE(X,Y,Z)	while((X)->next!=(struct data_s *)(Y)){	\
	data_ptr=(X)->next->next;	\
	free((X)->next->data);	\
	if((Z)==(X)->next){	\
		(Z)=(X);	\
	}	\
	free((X)->next);	\
	(X)->next=data_ptr;	\
}

struct bsdconv_instance *bsdconv_create(const char *);
void bsdconv_init(struct bsdconv_instance *);
void bsdconv_destroy(struct bsdconv_instance *);
int bsdconv(struct bsdconv_instance *);
char * bsdconv_error(void);

int loadcodec(struct bsdconv_codec_t *, char *, int);
void unloadcodec(struct bsdconv_codec_t *);

#endif
