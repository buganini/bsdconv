#include <errno.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct data_rt *from;
	struct data_rt *to;
	struct data_rt *cursor;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	int e;
	r->from=NULL;
	r->to=NULL;
	while(arg){
		DATA_FREE(ins, r->from);
		DATA_FREE(ins, r->to);
		r->from=str2data(arg->key, &e, ins);
		if(e){
			free(r);
			return e;
		}
		if(r->from==NULL){
			free(r);
			return EINVAL;
		}
		if(arg->ptr){
			r->to=str2data(arg->ptr, &e, ins);
			if(e){
				DATA_FREE(ins, r->from);
				free(r);
				return e;
			}
		}
		arg=arg->next;
	}
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	DATA_FREE(ins, r->from);
	DATA_FREE(ins, r->to);
	free(r);
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->cursor=r->from;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;
	unsigned char *datai=this_phase->curr->data;
	unsigned char *datar=r->cursor->data;
	size_t l=this_phase->curr->len;
	size_t i;

	if(l != r->cursor->len){
		r->cursor=r->from;
		this_phase->state.status=DEADEND;
		return;
	}

	for(i=0;i<l;i+=1){
		if(datai[i] != datar[i]){
			r->cursor=r->from;
			this_phase->state.status=DEADEND;
			return;
		}
	}

	if(r->cursor->next != NULL){
		r->cursor = r->cursor->next;
		this_phase->state.status=CONTINUE;
		return;
	}else{
		r->cursor = r->from;
		LISTCPY(ins, this_phase->data_tail, r->to);
		this_phase->state.status=NEXTPHASE;
		return;
	}
}
