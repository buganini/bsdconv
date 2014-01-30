#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_filter *filter;
	int limit;
	int passed;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv=r;
	r->filter=NULL;
	r->limit=0;
	int i;

	char *filter=NULL;
	while(arg){
		if(strcasecmp(arg->key, "FOR")==0){
			filter=arg->ptr;
		}else if(strcasecmp(arg->key, "LIMIT")==0 && sscanf(arg->ptr, "%d", &i)==1){
			r->limit=i;
		}else{
			free(r);
			return EINVAL;
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
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->passed=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	if(r->filter)
		unload_filter(r->filter);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	int pass=1;

	if(t->filter!=NULL && !t->filter->cbfilter(this_phase->curr)){
		pass=0;
	}

	if(pass && t->limit!=0){
		if(t->passed < t->limit){
			t->passed += 1;
		}else{
			pass=0;
		}
	}

	if(pass){
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		*(this_phase->data_tail)=*(this_phase->curr);
		this_phase->data_tail->next=NULL;
		this_phase->curr->flags &= ~F_FREE;

		this_phase->i=this_phase->curr->len-1;
		this_phase->state.status=NEXTPHASE;
	}else{
		this_phase->state.status=DEADEND;
	}

	return;
}
