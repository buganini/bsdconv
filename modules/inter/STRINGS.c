#include <errno.h>
#include "../../src/bsdconv.h"

struct my_s {
	struct bsdconv_filter *filter;
	struct data_rt *qh, *qt;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));

	char *filter="PRINT";
	while(arg){
		filter=arg->key;
		arg=arg->next;
	}

	r->filter=load_filter(filter);
	if(r->filter==NULL){
		free(r);
		return EOPNOTSUPP;
	}

	DATA_MALLOC(r->qh);
	r->qh->flags=0;
	r->qh->next=NULL;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	struct data_rt *t;
	while(r->qh->next){
		t=r->qh->next->next;
		DATUM_FREE(r->qh->next);
		r->qh->next=t;
	}
	r->qt=r->qh;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	struct data_rt *t;
	unload_filter(r->filter);
	while(r->qh){
		t=r->qh->next;
		DATUM_FREE(r->qh);
		r->qh=t;
	}
	free(r);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	if(r->qh->next){
		DATA_MALLOC(r->qt->next);
		r->qt=r->qt->next;
		r->qt->data="\x01\n";
		r->qt->len=2;
		r->qt->flags=0;
		r->qt->next=NULL;

		this_phase->data_tail->next=r->qh->next;
		this_phase->data_tail=r->qt;
		r->qh->next=NULL;
		r->qt=r->qh;
	}

	this_phase->state.status=NEXTPHASE;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	if(r->filter->cbfilter(this_phase->curr)){
		DATA_MALLOC(r->qt->next);
		r->qt=r->qt->next;
		*(r->qt)=*(this_phase->curr);
		this_phase->curr->flags &= ~F_FREE;
		r->qt->next=NULL;

		this_phase->state.status=SUBMATCH;
		return;
	}

	cbflush(ins);
}
