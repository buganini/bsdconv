#include "../../src/bsdconv.h"

#include "_NF-CCC.h"

struct ll_s{
	struct data_rt *p;
	int ccc;
	struct ll_s *prev;
	struct ll_s *next;
};
	/*
	 * here use a double linked list with tailed holder.
	*/

struct my_s{
	struct ll_s *head;
	struct ll_s *tail;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	r->tail=malloc(sizeof(struct ll_s));
	r->tail->next=NULL;
	r->head=r->tail;
	r->head->prev=NULL;
	return 0;
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	while(r->head->next){
		this_phase->data_tail->next=r->head->p;
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;

		struct ll_s *next=r->head->next;
		free(r->head);
		r->head=next;
	}
	r->head=r->tail;
	r->head->prev=NULL;

	this_phase->state.status=NEXTPHASE;
}

void cbconv(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	data=this_phase->curr->data;
	int i;
	int max=sizeof(ccc_table) / sizeof(struct ccc_interval) - 1;
	int min = 0;
	int mid;
	uint32_t ucs=0;
	int ccc=0;

	if(data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		if (ucs < ccc_table[0].beg || ucs > ccc_table[max].end){
			//noop
		}else while (max >= min) {
			mid = (min + max) / 2;
			if (ucs > ccc_table[mid].end)
				min = mid + 1;
			else if (ucs < ccc_table[mid].beg)
				max = mid - 1;
			else{
				ccc = ccc_table[mid].ccc;
				break;
			}
		}
	}

	if(ccc==0){
		while(r->head->next){
			this_phase->data_tail->next=r->head->p;
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;

			struct ll_s *next=r->head->next;
			free(r->head);
			r->head=next;
		}
		r->head=r->tail;
		r->head->prev=NULL;

		this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;

		this_phase->state.status=NEXTPHASE;
		return;
	}else{
		struct ll_s *next=r->tail;
		struct ll_s *p=r->tail->prev;
		while(p && ccc < p->ccc){
			next=p;
			p=p->prev;
		}
		struct ll_s *prev=next->prev;
		next->prev=malloc(sizeof(struct ll_s));
		next->prev->ccc=ccc;
		next->prev->prev=prev;
		next->prev->next=next;
		if(prev)
			prev->next=next->prev;
		if(r->head->prev)
			r->head=r->head->prev;
		next->prev->p=dup_data_rt(ins, this_phase->curr);

		this_phase->state.status=SUBMATCH;
		return;
	}
}


void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r->head);
	free(r);
}
