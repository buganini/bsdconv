#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_instance *ins;
	char ambi_width;
	size_t width;
	long remain;
	bsdconv_counter_t *full;
	bsdconv_counter_t *half;
	bsdconv_counter_t *ambi;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	int i;
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));

	char width_set=0;
	r->ambi_width=1;

	while(arg){
		if(strcasecmp(arg->key, "AMBI-AS-WIDE")==0 || strcasecmp(arg->key, "AMBIGUOUS-AS-WIDE")==0){
			r->ambi_width=2;
		}else if(sscanf(arg->key,"%d", &i)==1){
			r->width=i;
			width_set=1;
		}else{
			return EINVAL;
		}
		arg=arg->next;
	}

	if(width_set==0)
		return EINVAL;

	r->ins=bsdconv_create("WIDTH");
	r->full=bsdconv_counter(r->ins, "FULL");
	r->half=bsdconv_counter(r->ins, "HALF");
	r->ambi=bsdconv_counter(r->ins, "AMBI");
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	bsdconv_init(r->ins);
	r->remain=r->width;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	bsdconv_destroy(r->ins);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	bsdconv_counter_reset(r->ins, NULL);
	bsdconv_init(r->ins);
	r->ins->input=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	r->ins->input.next=NULL;
	r->ins->flush=1;
	bsdconv(r->ins);
	int w=*(r->full)*2 + *(r->half) + *(r->ambi) * r->ambi_width;
	if(r->remain >= w){
		this_phase->data_tail->next=r->ins->phase[r->ins->phasen].data_head->next;
		while(this_phase->data_tail->next){
			this_phase->data_tail=this_phase->data_tail->next;
		}
		r->ins->phase[r->ins->phasen].data_head->next=NULL;
		r->ins->phase[r->ins->phasen].data_tail=r->ins->phase[r->ins->phasen].data_head;
		r->remain -= w;
	}else{
		r->remain=-1;
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
