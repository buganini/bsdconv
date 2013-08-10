#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	int filter;
	int limit;
	int passed;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->filter=0;
	r->limit=0;
	while(arg){
		if(strcmp(arg->key, "FOR")==0){
			if(strcmp(arg->ptr, "UNICODE")==0 || strcmp(arg->ptr, "1")==0 || strcmp(arg->ptr, "01")==0){
				r->filter=1;
			}else if(strcmp(arg->ptr, "CNS11643")==0 || strcmp(arg->ptr, "2")==0 || strcmp(arg->ptr, "02")==0){
				r->filter=2;
			}else if(strcmp(arg->ptr, "BYTE")==0 || strcmp(arg->ptr, "3")==0 || strcmp(arg->ptr, "03")==0){
				r->filter=3;
			}else if(strcmp(arg->ptr, "ANSI")==0 || strcmp(arg->ptr, "1B")==0){
				r->filter=0x1b;
			}else{
				free(r);
				return EINVAL;
			}
		}else if(strcmp(arg->key, "LIMIT")==0){
			sscanf(arg->ptr, "%d", &r->limit);
		}else{
			free(r);
			return EINVAL;
		}
		arg=arg->next;
	}
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->passed=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	int pass=1;

	if(t->filter && (this_phase->curr->len==0 || (t->filter!=0 && UCP(this_phase->curr->data)[0]!=t->filter))){
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
