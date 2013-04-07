/*
 * Some code and table come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
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
#define HASHKEY "WHITESPACE"

struct my_s{
	struct data_rt *queue;
	struct data_rt **last;
	struct data_rt **dlast;
	struct bsdconv_phase *rerail;
	size_t offsetA;
	size_t offsetB;
};

int cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	struct my_s *t;
	if(bsdconv_hash_has(ins, HASHKEY)){
		t=bsdconv_hash_get(ins, HASHKEY);
	}else{
		t=malloc(sizeof(struct my_s));
		bsdconv_hash_set(ins, HASHKEY, t);
	}
	t->queue=NULL;
	CURRENT_CODEC(ins)->priv=t;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	t->rerail=CURRENT_PHASE(ins);
	CURRENT_CODEC(ins)->data_z=0;
	t->offsetA=0;
	t->offsetB=0;
	t->last=&t->queue;
	struct data_rt *q;
	while(t->queue){
		DATA_FREE((struct data_rt *)t->queue->data);
		q=t->queue;
		t->queue=t->queue->next;
		DATA_FREE(q);
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	struct data_rt *q;
	if(bsdconv_hash_has(ins, HASHKEY)){
		while(t->queue){
			DATA_FREE((struct data_rt *)t->queue->data);
			q=t->queue;
			t->queue=t->queue->next;
			DATA_FREE(q);
		}
		free(t);
		bsdconv_hash_del(ins, HASHKEY);
	}
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
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
		DATA_FREE(t->queue);
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
		DATA_FREE(t->queue);
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
