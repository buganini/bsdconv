#include <unistd.h>

enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,
	SUBMATCH,
	CALLBACK,
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

	int nfrom;
	int ninter;
	int nto;
	struct bsdconv_codec_t *from;
	struct bsdconv_codec_t *inter;
	struct bsdconv_codec_t *to;

	unsigned char pend_from, pend_inter, pend_to;

	unsigned char ierr, oerr;

	struct state_s from_state, inter_state, to_state;
	int from_index, inter_index, to_index;
	struct data_s *from_match, *inter_match, *to_match;
	unsigned char *from_bak;
	struct data_s *inter_bak, *to_bak;
	struct data_s *inter_data_head, *to_data_head, *out_data_head, *inter_data_tail, *to_data_tail, *out_data_tail;
	struct data_s *inter_data, *to_data;
	void **from_priv, **inter_priv, **to_priv;
};

struct bsdconv_codec_t {
	char *desc;
	int fd;
	unsigned char *z;
	size_t maplen;
	void *dl;
	void (*callback)(struct bsdconv_instance *);
	void *(*cbcreate)(void);
	void (*cbinit)(void *);
	void (*cbdestroy)(void *);
};

#define listcpy(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	ins->X##_data_tail->next=malloc(sizeof(struct data_s));	\
	ins->X##_data_tail=ins->X##_data_tail->next;	\
	memcpy(ins->X##_data_tail, (unsigned char *)((Z)+(uintptr_t)data_ptr), sizeof(struct data_s));	\
	data_ptr=ins->X##_data_tail->next;	\
	ins->X##_data_tail->next=NULL;	\
	ptr=(unsigned char *)((Z)+(uintptr_t)ins->X##_data_tail->data);	\
	ins->X##_data_tail->data=malloc(ins->X##_data_tail->len);	\
	memcpy(ins->X##_data_tail->data, ptr, ins->X##_data_tail->len);	\
}

#define listfree(X,Y)	while(ins->X##_data_head->next!=(struct data_s *)(Y)){	\
	data_ptr=ins->X##_data_head->next->next;	\
	free(ins->X##_data_head->next->data);	\
	if(ins->X##_data_tail==ins->X##_data_head->next){	\
		ins->X##_data_tail=ins->X##_data_head;	\
	}	\
	free(ins->X##_data_head->next);	\
	ins->X##_data_head->next=data_ptr;	\
}

struct bsdconv_instance *bsdconv_create(const char *);
void bsdconv_init(struct bsdconv_instance *);
void bsdconv_destroy(struct bsdconv_instance *);
int bsdconv(struct bsdconv_instance *);
char * bsdconv_error(void);
