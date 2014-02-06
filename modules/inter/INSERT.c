#include <errno.h>
#include  <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct data_rt *after;
	struct data_rt *before;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	int e;
	r->after=NULL;
	r->before=NULL;

	char *after=NULL;
	char *before=NULL;
	while(arg){
		if(strcasecmp(arg->key, "AFTER")==0){
			after=arg->ptr;
		}else if(strcasecmp(arg->key, "BEFORE")==0){
			before=arg->ptr;
		}else{
			return EINVAL;
		}
		arg=arg->next;
	}

	if(after){
		r->after=str2data(after, &e, ins);
		if(e){
			if(r->after)
				DATA_FREE(ins, r->after);
			free(r);
			return e;
		}
	}

	if(before){
		r->before=str2data(before, &e, ins);
		if(e){
			if(r->after)
				DATA_FREE(ins, r->after);
			if(r->before)
				DATA_FREE(ins, r->before);
			free(r);
			return e;
		}
	}

	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	if(r->after)
		DATA_FREE(ins, r->after);
	if(r->before)
		DATA_FREE(ins, r->before);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	if(r->before)
		LISTCPY(ins, this_phase->data_tail, r->before);

	this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	if(r->after)
		LISTCPY(ins, this_phase->data_tail, r->after);

	this_phase->state.status=NEXTPHASE;
	return;
}
