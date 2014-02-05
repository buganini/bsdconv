#ifndef BSDCONV_H
#define BSDCONV_H

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
 #include <dlfcn.h>
#endif

#if defined(__linux__)
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <endian.h>
#else
#include <sys/endian.h>
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

#define en_offset(X) htole32(X)
#define de_offset(X) le32toh(X)
#define en_uint16(X) htole16(X)
#define de_uint16(X) le16toh(X)
typedef uint32_t offset_t;
typedef size_t bsdconv_counter_t;

enum bsdconv_phase_type {
	_INPUT,
	FROM,
	INTER,
	TO,
	FILTER, //for convenient use in module functions
	SCORER, //for convenient use in module functions
};

#ifdef _BSDCONV_INTERNAL
enum bsdconv_status {
	CONTINUE,
	DEADEND,
	MATCH,
	SUBMATCH,
	SUBROUTINE,
	SUBMATCH_SUBROUTINE,
	NEXTPHASE,
	NOMATCH,
	NOOP,
	YIELD,
};
#endif

enum bsdconv_output_mode {
	BSDCONV_HOLD,
	BSDCONV_AUTOMALLOC,
	BSDCONV_PREMALLOCED,
	BSDCONV_FILE,
	BSDCONV_FD,
	BSDCONV_NULL,
	BSDCONV_PASS,
};

#ifdef _BSDCONV_INTERNAL
struct data_st {
	offset_t data;
	offset_t len;
	offset_t next;
};
#endif

struct data_rt {
	void *data;
	size_t len;
	struct data_rt *next;
	unsigned char flags;
};

struct state_st {
	unsigned char status;
	offset_t data;
	uint16_t beg;
	uint16_t end;
	offset_t base;
};

struct state_rt {
	unsigned char status;
	struct data_rt *data;
	uint16_t beg;
	uint16_t end;
	offset_t base;
};

#ifdef _BSDCONV_INTERNAL
struct bsdconv_hash_entry {
	char *key;
	void *ptr;
	struct bsdconv_hash_entry *next;
};
#endif

struct bsdconv_counter_entry {
	char *key;
	bsdconv_counter_t val;
	struct bsdconv_counter_entry *next;
};

struct bsdconv_instance {
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

struct bsdconv_phase {
	void *match_data;
	struct data_rt *bak, *data_head, *data_tail, *curr;
	struct state_rt state;
	int index;
	unsigned int i;
	struct bsdconv_codec *codec;
	int codecn;
	offset_t offset;
	char flags;
	char type;
};

#ifdef WIN32
#define SHAREOBJECT HMODULE
#define OPEN_SHAREOBJECT(path) LoadLibrary(path)
#define SHAREOBJECT_SYMBOL(so, symbol) ((void *)GetProcAddress(so, symbol))
#define CLOSE_SHAREOBJECT(path) FreeLibrary(path)
#else
#define SHAREOBJECT void *
#define OPEN_SHAREOBJECT(path) dlopen(path, RTLD_LAZY)
#define SHAREOBJECT_SYMBOL(so, symbol) dlsym(so, symbol)
#define CLOSE_SHAREOBJECT(so) dlclose(so)
#endif

struct bsdconv_filter {
	SHAREOBJECT so;
	int (*cbfilter)(struct data_rt *);
};

struct bsdconv_scorer {
	SHAREOBJECT so;
	int (*cbscorer)(struct data_rt *);
};

struct bsdconv_codec {
#ifdef WIN32
	HANDLE fd;
	HANDLE md;
#else
	int fd;
	size_t maplen;
#endif
	SHAREOBJECT dl;
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
#ifdef _BSDCONV_INTERNAL
struct bsdconv_filter *load_filter(const char *);
void unload_filter(struct bsdconv_filter *);

struct bsdconv_scorer *load_scorer(const char *);
void unload_scorer(struct bsdconv_scorer *);

#define LISTCPY_ST(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	struct data_st data_st; \
	DATA_MALLOC((X)->next);	\
	(X)=(X)->next;	\
	memcpy(&data_st, (char *)((Z)+(uintptr_t)data_ptr), sizeof(struct data_st));	\
	data_ptr=(void *)(uintptr_t)de_offset(data_st.next);	\
	(X)->data=(char *)((Z)+(uintptr_t)de_offset(data_st.data));	\
	(X)->len=data_st.len;	\
	(X)->flags=0; \
	(X)->next=NULL;	\
}

#define LISTCPY(X,Y) do{	\
	struct data_rt *data_ptr=(Y);	\
	while(data_ptr){	\
		DATA_MALLOC((X)->next);	\
		(X)=(X)->next;	\
		*(X)=*data_ptr;	\
		(X)->flags=0; \
		(X)->next=NULL;	\
		data_ptr=data_ptr->next;	\
	}	\
}while(0);

#define LISTFREE(X,Y,Z)	while((X)->next && (X)->next!=(Y)){	\
	data_ptr=(X)->next->next;	\
	DATUM_FREE((X)->next);	\
	if((Z)==(X)->next){	\
		(Z)=(X);	\
	}	\
	(X)->next=data_ptr;	\
}

static inline struct state_rt read_state(void *p){
	struct state_st state_st;
	struct state_rt state;
	memcpy(&state_st, p, sizeof(struct state_st));
	state.status=state_st.status;
	state.data=(void *)(uintptr_t)de_offset(state_st.data);
	state.beg=de_uint16(state_st.beg);
	state.end=de_uint16(state_st.end);
	state.base=de_offset(state_st.base);
	return state;
}

#define RESET(X) do{	\
	ins->phase[X].index=0;	\
	ins->phase[X].offset=0;	\
	ins->phase[X].state=read_state(ins->phase[X].codec[ins->phase[X].index].z);	\
}while(0)

#define CP(X) ((char *)(X))
#define UCP(X) ((unsigned char *)(X))

#define DATA_MALLOC(X) do{if(ins->pool){(X)=ins->pool; ins->pool=ins->pool->next;}else{(X)=malloc(sizeof(struct data_rt));}}while(0)
#define DATUM_FREE(X) do{ if((X)->flags & F_FREE) free((X)->data); (X)->next=ins->pool; ins->pool=(X);}while(0)
#define DATA_FREE(X) do{ struct data_rt *t,*p=(X); while(p){if(p->flags & F_FREE) free(p->data); t=p->next; p->next=ins->pool; ins->pool=p; p=t;}}while(0)

#define PREV_PHASE(INS) (&(INS)->phase[(INS)->phase_index-1])
#define LAST_PHASE(INS) (&(INS)->phase[(INS)->phasen])
#define THIS_PHASE(INS) (&(INS)->phase[(INS)->phase_index])
#define THIS_CODEC(INS) (&(INS)->phase[(INS)->phase_index].codec[(INS)->phase[(INS)->phase_index].index])
#endif

//API
//main
struct bsdconv_instance *bsdconv_create(const char *);
void bsdconv_init(struct bsdconv_instance *);
void bsdconv_ctl(struct bsdconv_instance *, int, void *, int);
void bsdconv_destroy(struct bsdconv_instance *);
void bsdconv(struct bsdconv_instance *);
char * bsdconv_error(void);
char *bsdconv_pack(struct bsdconv_instance *);

//counter
bsdconv_counter_t * bsdconv_counter(struct bsdconv_instance *, const char *);
void bsdconv_counter_reset(struct bsdconv_instance *, const char *);

//hash
void bsdconv_hash_set(struct bsdconv_instance *, const char *, void *);
void * bsdconv_hash_get(struct bsdconv_instance *, const char *);
int bsdconv_hash_has(struct bsdconv_instance *, const char *);
void bsdconv_hash_del(struct bsdconv_instance *, const char *);

//module
char * bsdconv_solve_alias(int, char *);
int bsdconv_module_check(int, const char *);
int bsdconv_codec_check(int, const char *);
char ** bsdconv_modules_list(int);
char ** bsdconv_codecs_list(int);

//util
int bsdconv_get_phase_index(struct bsdconv_instance *, int);
int bsdconv_get_codec_index(struct bsdconv_instance *, int, int);
char * bsdconv_insert_phase(const char *, const char *, int, int);
char * bsdconv_insert_codec(const char *, const char *, int, int);
char * bsdconv_replace_phase(const char *, const char *, int, int);
char * bsdconv_replace_codec(const char *, const char *, int, int);
void *bsdconv_malloc(size_t);
void bsdconv_free(void *);
int bsdconv_mkstemp(char *);
int str2datum(const char *, struct data_rt *);
struct data_rt * str2data(const char *, int *, struct bsdconv_instance *);
char * getCodecDir();

//Callback function interface
void cbconv(struct bsdconv_instance *);
void cbflush(struct bsdconv_instance *);
int cbcreate(struct bsdconv_instance *, struct bsdconv_hash_entry *);
void cbinit(struct bsdconv_instance *);
void cbctl(struct bsdconv_instance *, int, void *, size_t);
void cbdestroy(struct bsdconv_instance *);
int cbfilter(struct data_rt *data);

//CTL Action
enum bsdconv_ctl_action {
	BSDCONV_CTL_ATTACH_SCORE = 0,
	BSDCONV_CTL_ATTACH_OUTPUT_FILE = 3,
	BSDCONV_CTL_AMBIGUOUS_PAD = 4
};

//Helpers
static inline struct data_rt * dup_data_rt(struct bsdconv_instance *ins, struct data_rt *data){
	struct data_rt *ret;
	DATA_MALLOC(ret);
	*ret=*data;
	data->flags &= ~F_FREE;
	return ret;
}

static inline void strtoupper(char *s){
	char *c;
	for(c=s;*c;++c){
		if(*c>='a' && *c<='z'){
			*c=*c-'a'+'A';
		}
	}
}

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

// modules/filter/unicode_range.c
struct uint32_range {
	uint32_t first;
	uint32_t last;
};

// modules/score/unicode_range.c
struct uint32_range_with_score {
	uint32_t first;
	uint32_t last;
	uint32_t score;
};


#ifdef __cplusplus
}
#endif

#endif
