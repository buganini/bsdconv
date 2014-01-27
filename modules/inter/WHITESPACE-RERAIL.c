#include "../../src/bsdconv.h"
#define HASHKEY "WHITESPACE"

struct my_s{
	struct data_rt *queue;
	struct data_rt **last;
	struct data_rt **dlast;
	struct bsdconv_phase *rerail;
	size_t offsetA;
	size_t offsetB;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *t;
	if(bsdconv_hash_has(ins, HASHKEY)){
		t=bsdconv_hash_get(ins, HASHKEY);
	}else{
		t=malloc(sizeof(struct my_s));
		bsdconv_hash_set(ins, HASHKEY, t);
	}
	t->queue=NULL;
	THIS_CODEC(ins)->priv=t;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *t=THIS_CODEC(ins)->priv;
	t->rerail=THIS_PHASE(ins);
	t->offsetA=0;
	t->offsetB=0;
	t->last=&t->queue;
	struct data_rt *q;
	while(t->queue){
		DATUM_FREE((struct data_rt *)t->queue->data);
		q=t->queue;
		t->queue=t->queue->next;
		DATUM_FREE(q);
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *t=THIS_CODEC(ins)->priv;
	struct data_rt *q;
	if(bsdconv_hash_has(ins, HASHKEY)){
		while(t->queue){
			DATUM_FREE((struct data_rt *)t->queue->data);
			q=t->queue;
			t->queue=t->queue->next;
			DATUM_FREE(q);
		}
		free(t);
		bsdconv_hash_del(ins, HASHKEY);
	}
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	struct data_rt *q;

	this_phase->state.status=NEXTPHASE;

	while(t->queue && t->queue->len<=t->offsetB){
		this_phase->data_tail->next=t->queue->data;
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		if(&t->queue->next==t->last){
			t->last=&t->queue;
			t->dlast=NULL;
		}
		q=t->queue->next;
		DATUM_FREE(t->queue);
		t->queue=q;
	}

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;
	t->offsetB+=1;

	while(t->queue && t->queue->len<=t->offsetB){
		this_phase->data_tail->next=t->queue->data;
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		if(&t->queue->next==t->last){
			t->last=&t->queue;
			t->dlast=NULL;
		}
		q=t->queue->next;
		DATUM_FREE(t->queue);
		t->queue=q;
	}

	if(t->queue){
		this_phase->flags |= (F_MATCH | F_PENDING);
		this_phase->match_data=t->queue->data;
	}else{
		this_phase->flags &= ~(F_MATCH | F_PENDING);
		this_phase->match_data=NULL;
	}

	return;
}
