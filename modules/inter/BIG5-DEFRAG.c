#include "../../src/bsdconv.h"

struct my_s{
	struct data_rt *p;
	struct data_rt *q;
	struct data_rt **r;
	char f;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->p=NULL;
	r->q=NULL;
	r->r=&(r->q);
	r->f=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	free(THIS_CODEC(ins)->priv);
}

void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;
	data=this_phase->curr->data;

	if(r->f==0){
		if(data[0]==0x3 && data[1]>0x7f){
			r->f=1;
			r->p=dup_data_rt(ins, this_phase->curr);
			this_phase->state.status=SUBMATCH;
			return;
		}else{
			DATA_MALLOC(ins, this_phase->data_tail->next);
			this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			return;
		}
	}else if(r->f){
		if(data[0]==0x1b){
			*(r->r)=dup_data_rt(ins, this_phase->curr);
			(*(r->r))->next=NULL;
			r->r=&((*(r->r))->next);

			this_phase->state.status=SUBMATCH;
			return;
		}else{
			r->f=0;

			this_phase->data_tail->next=r->p;
			this_phase->data_tail=this_phase->data_tail->next;

			this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
			this_phase->data_tail=this_phase->data_tail->next;

			if(r->q){
				this_phase->data_tail->next=r->q;
				*(r->r)=NULL;
				while(this_phase->data_tail->next){
					this_phase->data_tail=this_phase->data_tail->next;
				}
			}
			r->p=r->q=NULL;
			r->r=&(r->q);
			r->f=0;
			this_phase->state.status=NEXTPHASE;
			return;
		}
	}
}
