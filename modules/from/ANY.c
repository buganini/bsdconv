#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_st {
	struct data_rt *data;
	bsdconv_counter_t *counter;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_st *r=malloc(sizeof(struct my_st));
	struct data_rt *bak;
	int e;
	r->data=str2data("013F", &e, ins);
	r->counter=NULL;
	while(arg){
		if(strcasecmp(arg->key, "ERROR")==0){
			if(arg->ptr)
				r->counter=bsdconv_counter(ins, arg->ptr);
			else
				r->counter=bsdconv_counter(ins, "IERR");
		}else{
			bak=r->data;
			r->data=str2data(arg->key, &e, ins);
			DATA_FREE(ins, bak);
			if(e){
				DATA_FREE(ins, r->data);
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
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_st *r=this_phase->codec[this_phase->index].priv;
	DATA_FREE(ins, r->data);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_st *r=this_phase->codec[this_phase->index].priv;

	LISTCPY(ins, this_phase->data_tail, r->data);

	this_phase->state.status=NEXTPHASE;

	if(r->counter)
		*(r->counter)+=1;
	return;
}
