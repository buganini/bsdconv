#include <stdio.h>
#include <errno.h>
#include "../../src/bsdconv.h"

struct my_s {
	struct bsdconv_filter *filter;
	struct data_rt *qh, *qt;
	unsigned int acc_len;
	int min_len;
	struct data_rt *sep;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	int i;

	r->min_len=1;

	char *filter="PRINT";
	char *sep="010A";
	while(arg){
		if(strcasecmp(arg->key, "FOR")==0){
			filter=arg->ptr;
		}else if(strcasecmp(arg->key, "MIN-LEN")==0 && sscanf(arg->ptr, "%d", &i)==1){
			r->min_len=i;
		}else if(strcasecmp(arg->key, "SEP")==0){
			sep=arg->ptr;
		}else{
			free(r);
			return EINVAL;
		}
		arg=arg->next;
	}

	r->filter=load_filter(filter);
	if(r->filter==NULL){
		free(r);
		return EOPNOTSUPP;
	}

	r->sep=str2data(sep, &i, ins);
	if(i){
		DATA_FREE(r->sep);
		if(r->filter)
			unload_filter(r->filter);
		free(r);
		return i;
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
	r->acc_len=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	struct data_rt *t;
	DATA_FREE(r->sep);
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
		if(r->acc_len >= r->min_len){
			LISTCPY(r->qt, r->sep);

			this_phase->data_tail->next=r->qh->next;
			this_phase->data_tail=r->qt;
			r->qh->next=NULL;
			r->qt=r->qh;
			r->acc_len=0;
		}else{
			DATA_FREE(r->qh->next);
			r->qt=r->qh;
			r->acc_len=0;
		}
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
		r->acc_len+=1;

		this_phase->state.status=SUBMATCH;
		return;
	}

	cbflush(ins);
}
