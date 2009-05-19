#include <unistd.h>

struct bsdconv_t {
	int nfrom;
	int ninter;
	int nto;
	struct bsdconv_codec_t *from;
	struct bsdconv_codec_t *inter;
	struct bsdconv_codec_t *to;
};

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

struct bsdconv_instruction{
	unsigned char *in_buf;
	size_t in_len;
	unsigned char *out_buf;
	size_t out_len;
	unsigned char *feed;
	size_t feed_len;
	unsigned char *back;
	size_t back_len;
	unsigned char *from_data;

	unsigned char pend_from, pend_inter, pend_to;

	unsigned char ierr, oerr;

	struct state_s from_state, inter_state, to_state;
	int from_index, inter_index, to_index;
	struct data_s *from_match, *inter_match, *to_match;
	unsigned char *from_bak;
	struct data_s *inter_bak, *to_bak;
	struct data_s inter_data_ent, to_data_ent, out_data_ent;
	struct data_s *inter_data_head, *to_data_head, *out_data_head, *inter_data_tail, *to_data_tail, *out_data_tail;
	struct data_s *inter_data, *to_data;
	void **fpriv, **ipriv, **tpriv;
};

struct bsdconv_codec_t {
	char *desc;
	int fd;
	unsigned char *z;
	void *dl;
	void (*callback)(struct bsdconv_instruction *);
	void *(*cbinit)(void);
	void (*cbclear)(void *);
};

#define listcpy(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	ins->X##_data_tail->next=malloc(sizeof(struct data_s));	\
	ins->X##_data_tail=ins->X##_data_tail->next;	\
	memcpy(ins->X##_data_tail, (unsigned char *)((Z)+(unsigned int)data_ptr), sizeof(struct data_s));	\
	data_ptr=ins->X##_data_tail->next;	\
	ins->X##_data_tail->next=NULL;	\
	ptr=(unsigned char *)((Z)+(unsigned int)ins->X##_data_tail->data);	\
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

void bsdconv_init(struct bsdconv_t *, struct bsdconv_instruction *, unsigned char *, size_t, unsigned char *, size_t);
struct bsdconv_t *bsdconv_create(const char *);
void bsdconv_destroy(struct bsdconv_t *);
int bsd_conv(struct bsdconv_t *, struct bsdconv_instruction *);
