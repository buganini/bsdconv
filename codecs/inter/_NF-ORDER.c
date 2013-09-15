/*
 * Some code and table come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 * Copyright (c) 2009-2013 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "../../src/bsdconv.h"

#include "_NF_CCC.h"

struct ll_s{
	struct data_rt *p;
	int ccc;
	struct ll_s *next;
};

struct my_s{
	struct ll_s *head;
	struct ll_s *tail;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	r->head=malloc(sizeof(struct ll_s));
	r->head->next=NULL;
	r->tail=r->head;
	return 0;
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;

	while(r->head->next){
		this_phase->data_tail->next=r->head->next->p;
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;

		struct ll_s *next=r->head->next->next;
		free(r->head->next);
		r->head->next=next;
	}
	r->tail=r->head;

	this_phase->state.status=NEXTPHASE;
}

void cbconv(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	unsigned char *data;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
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
			this_phase->data_tail->next=r->head->next->p;
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;

			struct ll_s *next=r->head->next->next;
			free(r->head->next);
			r->head->next=next;
		}
		r->tail=r->head;

		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		*(this_phase->data_tail)=*(this_phase->curr);
		this_phase->curr->flags &= ~F_FREE;
		this_phase->data_tail->next=NULL;

		this_phase->state.status=NEXTPHASE;
		return;
	}else{
		struct ll_s *prev=r->head;
		struct ll_s *p=r->head->next;
		while(p && ccc >= p->ccc){
			prev=p;
			p=p->next;
		}
		struct ll_s *next=prev->next;
		prev->next=malloc(sizeof(struct ll_s));
		prev->next->ccc=ccc;
		prev->next->next=next;
		if(r->tail->next)
			r->tail=r->tail->next;
		DATA_MALLOC(prev->next->p);
		*(prev->next->p)=*(this_phase->curr);
		this_phase->curr->flags &= ~F_FREE;

		this_phase->state.status=SUBMATCH;
		return;
	}
}


void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r->head);
	free(r);
}
