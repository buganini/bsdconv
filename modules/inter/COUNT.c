#include <errno.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_filter *filter;
	bsdconv_counter_t *counter;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv=r;
	r->filter=NULL;

	char *filter=NULL;
	char *key="COUNT";
	while(arg){
		if(strcasecmp(arg->key, "FOR")==0){
			filter=arg->ptr;
		}else{
			key=arg->key;
		}
		arg=arg->next;
	}
	if(filter!=NULL){
		r->filter=load_filter(filter);
		if(r->filter==NULL){
			free(r);
			return EOPNOTSUPP;
		}
	}
	r->counter=bsdconv_counter(ins, key);
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	if(r->filter)
		unload_filter(r->filter);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	if(r->filter==NULL || r->filter->cbfilter(this_phase->curr))
		*(r->counter)+=1;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	this_phase->state.status=NEXTPHASE;
	return;
}
