#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status; //in MBCS mode
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->status=0;
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;

	this_phase->flags &= ~F_PENDING;
	if(t->status!=0){
		t->status=0;

		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=1;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(1);
		*CP(this_phase->data_tail->data)='\x0F';
	}

	this_phase->state.status=NEXTPHASE;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phase_index-1];
	struct my_s *t=THIS_CODEC(ins)->priv;
	struct data_st data;
	struct data_rt *data_ptr;

	memcpy(&data, (char *)(this_phase->codec[this_phase->index].data_z+(uintptr_t)this_phase->state.data), sizeof(struct data_st));

	if(data.len>1 && t->status==0){
		t->status=1;
		this_phase->flags |= F_PENDING;

		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=1;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(1);
		*CP(this_phase->data_tail->data)='\x0E';
	}else if(data.len==1 && t->status!=0){
		t->status=0;
		this_phase->flags &= ~F_PENDING;

		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=1;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(1);
		*CP(this_phase->data_tail->data)='\x0F';
	}

	if(t->status){
		LISTCPY_ST(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

		this_phase->data_head->len=0;
		this_phase->flags |= (F_MATCH | F_PENDING);
		this_phase->match_data=NULL;

		this_phase->bak=this_phase->curr->next;
		LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
		this_phase->curr=prev_phase->data_head;

		RESET(ins->phase_index);

		ins->phase_index+=1;

		this_phase->state.status=NOOP;
	}else{
		LISTCPY_ST(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

		this_phase->state.status=NEXTPHASE;
	}
}
