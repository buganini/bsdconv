/*
 * Copyright (c) 2009-2013 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//struct data_rt.flags
#define F_FREE 1
#define F_MARK 2

//struct bsdconv_phase.flags
#define F_MATCH 1
#define F_PENDING 2
#define F_LOOPBACK 4

typedef uint32_t offset_t;
typedef size_t bsdconv_counter_t;

enum bsdconv_phase_type {
	_INPUT,
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
	SUBMATCH_SUBROUTINE,
	NEXTPHASE,
	NOMATCH,
	NOOP
};

enum bsdconv_output_mode{
	BSDCONV_HOLD,
	BSDCONV_AUTOMALLOC,
	BSDCONV_PREMALLOCED,
	BSDCONV_FILE,
	BSDCONV_FD,
	BSDCONV_NULL,
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
	unsigned char flags;
};

struct state_st{
	char status;
	struct data_st *data;
	uint16_t beg;
	uint16_t end;
	offset_t base;
};

struct state_rt{
	char status;
	struct data_rt *data;
	uint16_t beg;
	uint16_t end;
	offset_t base;
};

struct bsdconv_hash_entry{
	char *key;
	void *ptr;
	struct bsdconv_hash_entry *next;
};

struct bsdconv_counter_entry{
	char *key;
	bsdconv_counter_t val;
	struct bsdconv_counter_entry *next;
};

struct bsdconv_instance{
	int output_mode;

	struct data_rt input, output;

	char flush;

	struct bsdconv_phase *phase;
	int phasen, phase_index;
	struct bsdconv_hash_entry *hash;
	struct bsdconv_counter_entry *counter;

	bsdconv_counter_t *ierr;
	bsdconv_counter_t *oerr;

	struct data_rt *pool;
};

struct bsdconv_phase{
	struct data_rt *bak, *match_data, *data_head, *data_tail, *curr;
	struct state_rt state;
	int index;
	unsigned int i;
	struct bsdconv_codec_t *codec;
	int codecn;
	offset_t offset;
	char flags;
	char type;
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
	char *argv;
	char *z;
	char *data_z;
	char *desc;
	void (*cbconv)(struct bsdconv_instance *);
	void (*cbflush)(struct bsdconv_instance *);
	int (*cbcreate)(struct bsdconv_instance *, struct bsdconv_hash_entry *arg);
	void (*cbinit)(struct bsdconv_instance *);
	void (*cbctl)(struct bsdconv_instance *, int, void *, size_t);
	void (*cbdestroy)(struct bsdconv_instance *);
	void *priv;
};

#ifndef EDOOFUS
#define EDOOFUS 88
#endif

#ifdef WIN32
#define EOPNOTSUPP ERROR_NOT_SUPPORTED
#define ENOMEM ERROR_NOT_ENOUGH_MEMORY
#define EINVAL ERROR_BAD_COMMAND
#define SHLIBEXT "dll"
#define REALPATH(path, buf) GetFullPathName(path, PATH_MAX+1, buf, NULL)
char * strsep(char **, const char *);
char * index(const char *, int);
char * getwd(char *);
#else
#define SetLastError(n) errno=n
#define GetLastError() errno
#define SHLIBEXT "so"
#define REALPATH(path, buf) realpath(path, buf)
#endif


//Internal API
#define LISTCPY(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	DATA_MALLOC((X)->next);	\
	(X)=(X)->next;	\
	memcpy((X), (char *)((Z)+(uintptr_t)data_ptr), sizeof(struct data_st));	\
	data_ptr=(void *)((X)->next);	\
	(X)->next=NULL;	\
	(X)->data=(char *)((Z)+(uintptr_t)(X)->data);	\
	(X)->flags=0; \
}

#define LISTFREE(X,Y,Z)	while((X)->next && (X)->next!=(Y)){	\
	data_ptr=(X)->next->next;	\
	DATA_FREE((X)->next);	\
	if((Z)==(X)->next){	\
		(Z)=(X);	\
	}	\
	(X)->next=data_ptr;	\
}

#define RESET(X) do{	\
	ins->phase[X].index=0;	\
	ins->phase[X].offset=0;	\
	memcpy(&ins->phase[X].state, ins->phase[X].codec[ins->phase[X].index].z, sizeof(struct state_st));	\
}while(0)

#define CP(X) ((char *)(X))
#define UCP(X) ((unsigned char *)(X))

#define DATA_MALLOC(X) do{if(ins->pool){(X)=ins->pool; ins->pool=ins->pool->next;}else{(X)=malloc(sizeof(struct data_rt));}}while(0)
#define DATA_FREE(X) do{ if((X)->flags & F_FREE) free((X)->data); (X)->next=ins->pool; ins->pool=(X);}while(0)

#define CURRENT_PHASE(INS) (&(INS)->phase[(INS)->phase_index])
#define CURRENT_CODEC(INS) (&(INS)->phase[(INS)->phase_index].codec[(INS)->phase[(INS)->phase_index].index])

//API
struct bsdconv_instance *bsdconv_create(const char *);
int bsdconv_get_phase_index(struct bsdconv_instance *, int);
int bsdconv_get_codec_index(struct bsdconv_instance *, int, int);
char * bsdconv_insert_phase(const char *, const char *, int, int);
char * bsdconv_insert_codec(const char *, const char *, int, int);
char * bsdconv_replace_phase(const char *, const char *, int, int);
char * bsdconv_replace_codec(const char *, const char *, int, int);
void bsdconv_init(struct bsdconv_instance *);
void bsdconv_ctl(struct bsdconv_instance *, int, void *, int);
void bsdconv_destroy(struct bsdconv_instance *);
void bsdconv(struct bsdconv_instance *);
char * bsdconv_error(void);
bsdconv_counter_t * bsdconv_counter(struct bsdconv_instance *, const char *);
void bsdconv_counter_reset(struct bsdconv_instance *, const char *);
void bsdconv_hash_set(struct bsdconv_instance *, const char *, void *);
void * bsdconv_hash_get(struct bsdconv_instance *, const char *);
int bsdconv_hash_has(struct bsdconv_instance *, const char *);
void bsdconv_hash_del(struct bsdconv_instance *, const char *);
char * bsdconv_solve_alias(int, char *);
int bsdconv_codec_check(int, const char *);
char ** bsdconv_codecs_list(int);
char *bsdconv_pack(struct bsdconv_instance *);
void *bsdconv_malloc(size_t);
void bsdconv_free(void *);
int bsdconv_mkstemp(char *);
int str2datum(const char *, struct data_st *);
struct data_st * str2data(const char *, int *);
void free_data_st(struct data_st *);
char * getCodecDir();

//Callback function interface
void cbconv(struct bsdconv_instance *);
void cbflush(struct bsdconv_instance *);
int cbcreate(struct bsdconv_instance *, struct bsdconv_hash_entry *arg);
void cbinit(struct bsdconv_instance *);
void cbctl(struct bsdconv_instance *, int, void *, size_t);
void cbdestroy(struct bsdconv_instance *);

//CTL Action
enum bsdconv_ctl_action {
	BSDCONV_ATTACH_SCORE,
	BSDCONV_SET_WIDE_AMBI,
	BSDCONV_SET_TRIM_WIDTH,
	BSDCONV_ATTACH_OUTPUT_FILE,
	BSDCONV_AMBIGUOUS_PAD
};

//Binary
#define bb00000011 0x03
#define bb00000111 0x07
#define bb00001111 0x0f
#define bb00011100 0x1c
#define bb00110000 0x30
#define bb00111100 0x3c
#define bb00111111 0x3f
#define bb10000000 0x80
#define bb11000000 0xc0
#define bb11011000 0xd8
#define bb11011100 0xdc
#define bb11100000 0xe0
#define bb11110000 0xf0
#define bb11111000 0xf8
#define bb11111100 0xfc

#ifdef USE_OCT_MAP
int oct[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#endif
#ifdef USE_DEC_MAP
int dec[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#endif
#ifdef USE_HEX_MAP
int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#endif

#ifdef __cplusplus
}
#endif

#endif
