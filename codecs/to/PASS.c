#include <errno.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	int filter;
	int mark;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->filter=0;
	r->mark=0;
	while(arg){
		if(strcasecmp(arg->key, "MARK")==0){
			r->mark=1;
		}else if(strcasecmp(arg->key, "FOR")==0){
			if(strcasecmp(arg->ptr, "UNICODE")==0 || strcasecmp(arg->ptr, "1")==0 || strcasecmp(arg->ptr, "01")==0){
				r->filter=1;
			}else if(strcasecmp(arg->ptr, "CNS11643")==0 || strcasecmp(arg->ptr, "2")==0 || strcasecmp(arg->ptr, "02")==0){
				r->filter=2;
			}else if(strcasecmp(arg->ptr, "BYTE")==0 || strcasecmp(arg->ptr, "3")==0 || strcasecmp(arg->ptr, "03")==0){
				r->filter=3;
			}else if(strcasecmp(arg->ptr, "ANSI")==0 || strcasecmp(arg->ptr, "1B")==0){
				r->filter=0x1b;
			}else{
				free(r);
				return EINVAL;
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
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	int pass=1;

	if(t->filter!=0 && (this_phase->curr->len==0 || UCP(this_phase->curr->data)[0]!=t->filter))
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
