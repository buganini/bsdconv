#define USE_HEX_MAP

#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define F_CLEAR 0
#define F_A 1
#define F_B 2

struct my_s {
	struct data_rt data;
	/* extend struct data_rt */
	size_t size;
	char flag;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	r->data.data=NULL;
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *t=THIS_CODEC(ins)->priv;
	t->data.len=0;
	if(t->data.data)
		free(t->data.data);
	t->data.next=NULL;
	t->flag=F_CLEAR;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *t=THIS_CODEC(ins)->priv;
	if(t->data.data){
		free(t->data.data);
	}
	free(t);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	LISTCPY(this_phase->data_tail, &t->data);
}

void cbconv(struct bsdconv_instance *ins){
	struct data_rt *data_ptr;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phase_index-1];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	char d=CP(this_phase->curr->data)[this_phase->i];
	if(hex[(unsigned char)d]==-1){
		if(this_phase->flags & F_MATCH){
			this_phase->flags &= ~(F_PENDING | F_MATCH | F_LOOPBACK);

			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->data=t->data.data;
			this_phase->data_tail->len=t->data.len;
			this_phase->data_tail->flags=F_FREE;

			LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
			this_phase->curr=prev_phase->data_head;
			this_phase->i=this_phase->data_head->len;
			RESET(ins->phase_index);

			this_phase->state.status=NOOP;
		}else{
			this_phase->state.status=DEADEND;
			if(t->data.data) free(t->data.data);
		}
		t->data.data=NULL;
		t->flag=F_CLEAR;
	}else{
		if(t->flag==F_CLEAR){
			t->flag=F_A;
			t->data.len=0;
			t->data.data=NULL;
			t->size=0;
		}

		if(t->data.len)
			this_phase->state.status=SUBMATCH;
		else
			this_phase->state.status=CONTINUE;

		switch(t->flag){
			case F_A:
				if(t->data.len >= t->size){
					t->size+=8;
					t->data.data=realloc(t->data.data,t->size);
				}
				CP(t->data.data)[t->data.len]=hex[(unsigned char)d];
				t->data.len+=1;
				t->flag=F_B;
				break;
			case F_B:
				CP(t->data.data)[t->data.len-1]<<=4;
				CP(t->data.data)[t->data.len-1]|=hex[(unsigned char)d];
				t->flag=F_A;
				break;
		}
	}
}
