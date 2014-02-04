#include "../../src/bsdconv.h"
#include "WHITESPACE.h"

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *t;
	if(bsdconv_hash_has(ins, HASHKEY)){
		t=bsdconv_hash_get(ins, HASHKEY);
	}else{
		t=malloc(sizeof(struct my_s));
		bsdconv_hash_set(ins, HASHKEY, t);
	}
	t->queue=NULL;
	t->rerail=NULL;
	THIS_CODEC(ins)->priv=t;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *t=THIS_CODEC(ins)->priv;
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
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	struct data_rt *q;
	data=this_phase->curr->data;
	int i;
	uint32_t ucs=0;

	this_phase->state.status=NEXTPHASE;

	if(this_phase->curr->len>0 && data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		if(ucs==0x09||ucs==0x0A||ucs==0x0D||ucs==0x20){
			DATA_MALLOC(q);
			*(t->last)=q;
			q->next=NULL;
			q->flags=0;
			t->last=&q->next;
			q->data=(void *) dup_data_rt(ins, this_phase->curr);
			((struct data_rt *)q->data)->next=NULL;
			q->len=t->offsetA;

			if(t->rerail){
				t->rerail->flags |= (F_MATCH | F_PENDING);
				t->rerail->match_data = NULL;
			}

			return;
		}
	}
	t->offsetA+=1;

	this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	return;
}
