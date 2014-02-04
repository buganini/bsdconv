#include <stdio.h>
#include <errno.h>
#include "../../src/bsdconv.h"

struct my_s {
	struct bsdconv_filter *filter;
	struct data_rt *qh, *qt;
	unsigned int acc_len;
	int min_len;
	struct data_rt *after;
	struct data_rt *before;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	int i;

	r->min_len=1;
	r->after=NULL;
	r->before=NULL;

	char *filter="PRINT";
	char *after="010A";
	char *before=NULL;
	while(arg){
		if(strcasecmp(arg->key, "FOR")==0){
			filter=arg->ptr;
		}else if(strcasecmp(arg->key, "MIN-LEN")==0 && sscanf(arg->ptr, "%d", &i)==1){
			r->min_len=i;
		}else if(strcasecmp(arg->key, "AFTER")==0){
			after=arg->ptr;
		}else if(strcasecmp(arg->key, "BEFORE")==0){
			before=arg->ptr;
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

	if(after){
		r->after=str2data(after, &i, ins);
		if(i){
			if(r->after)
				DATA_FREE(r->after);
			free(r);
			return i;
		}
	}

	if(before){
		r->before=str2data(before, &i, ins);
		if(i){
			if(r->after)
				DATA_FREE(r->after);
			if(r->before)
				DATA_FREE(r->before);
			free(r);
			return i;
		}
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
	if(r->after)
		DATA_FREE(r->after);
	if(r->before)
		DATA_FREE(r->before);
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
			if(r->before)
				LISTCPY(this_phase->data_tail, r->before);

			this_phase->data_tail->next=r->qh->next;
			this_phase->data_tail=r->qt;
			r->qh->next=NULL;
			r->qt=r->qh;
			r->acc_len=0;

			if(r->after)
				LISTCPY(this_phase->data_tail, r->after);
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
		r->qt->next=dup_data_rt(ins, this_phase->curr);
		r->qt=r->qt->next;
		r->qt->next=NULL;
		r->acc_len+=1;

		this_phase->state.status=SUBMATCH;
		return;
	}

	cbflush(ins);
}
