#include <errno.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_filter *filter;
	int mark;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->filter=NULL;
	r->mark=0;
	while(arg){
		if(strcasecmp(arg->key, "MARK")==0){
			r->mark=1;
		}else if(strcasecmp(arg->key, "FOR")==0){
			r->filter=load_filter(arg->ptr);
			if(r->filter==NULL){
				free(r);
				return EOPNOTSUPP;
			}
		}else{
			free(r);
			return EINVAL;
		}
		arg=arg->next;
	}
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	if(r->filter)
		unload_filter(r->filter);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	int pass=1;

	if(t->filter!=NULL && !t->filter->cbfilter(this_phase->curr))
		pass=0;

	if(pass){
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		*(this_phase->data_tail)=*(this_phase->curr);
		this_phase->data_tail->next=NULL;
		this_phase->curr->flags &= ~F_FREE;

		if(t->mark)
			this_phase->data_tail->flags |= F_MARK;

		this_phase->state.status=NEXTPHASE;
	}else{
		this_phase->state.status=DEADEND;
	}

	return;
}
