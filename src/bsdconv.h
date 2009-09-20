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
	unsigned char *data;
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
	unsigned char *in_buf;
	size_t in_len;
	unsigned char *out_buf;
	size_t out_len;
	unsigned char *feed;
	size_t feed_len;
	unsigned char *back;
	size_t back_len;
	unsigned char *from_data;

	struct bsdconv_phase *phase;
	int phasen, phase_index;
	unsigned int ierr, oerr;
	unsigned char *from_bak;

};

struct bsdconv_phase{
	struct data_s *bak, *match, *data_head, *data_tail, *data;
	struct state_s state;
	int index;
	unsigned char pend;
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
	unsigned char *z;
	unsigned char *data_z;
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

#define listcpy(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	ins->phase[X].data_tail->next=malloc(sizeof(struct data_s));	\
	ins->phase[X].data_tail=ins->phase[X].data_tail->next;	\
	memcpy(ins->phase[X].data_tail, (unsigned char *)((Z)+(uintptr_t)data_ptr), sizeof(struct data_s));	\
	data_ptr=ins->phase[X].data_tail->next;	\
	ins->phase[X].data_tail->next=NULL;	\
	ptr=(unsigned char *)((Z)+(uintptr_t)ins->phase[X].data_tail->data);	\
	ins->phase[X].data_tail->data=malloc(ins->phase[X].data_tail->len);	\
	memcpy(ins->phase[X].data_tail->data, ptr, ins->phase[X].data_tail->len);	\
}

#define listfree(X,Y)	while(ins->phase[X].data_head->next!=(struct data_s *)(Y)){	\
	data_ptr=ins->phase[X].data_head->next->next;	\
	free(ins->phase[X].data_head->next->data);	\
	if(ins->phase[X].data_tail==ins->phase[X].data_head->next){	\
		ins->phase[X].data_tail=ins->phase[X].data_head;	\
	}	\
	free(ins->phase[X].data_head->next);	\
	ins->phase[X].data_head->next=data_ptr;	\
}

struct bsdconv_instance *bsdconv_create(const char *);
void bsdconv_init(struct bsdconv_instance *);
void bsdconv_destroy(struct bsdconv_instance *);
int bsdconv(struct bsdconv_instance *);
char * bsdconv_error(void);

int loadcodec(struct bsdconv_codec_t *, char *, int);
void unloadcodec(struct bsdconv_codec_t *);

#endif
