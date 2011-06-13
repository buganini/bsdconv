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

void cbcreate(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=bsdconv_hash(ins, HASHKEY, sizeof(struct my_s));
	t->queue=NULL;
	ins->phase[ins->phase_index].codec[this_phase->index].priv=t;
}

void cbinit(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=ins->phase[ins->phase_index].codec[this_phase->index].priv;
	t->offsetA=0;
	t->offsetB=0;
	t->last=&t->queue;
	t->dlast=NULL;
	struct data_rt *q;
	while(t->queue){
		DATA_FREE((struct data_rt *)t->queue->data);
		q=t->queue;
		t->queue=t->queue->next;
		DATA_FREE(q);
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=ins->phase[ins->phase_index].codec[this_phase->index].priv;
	struct data_rt *q;
	if(bsdconv_hash_has(ins, HASHKEY)){
		while(t->queue){
			DATA_FREE((struct data_rt *)t->queue->data);
			q=t->queue;
			t->queue=t->queue->next;
			DATA_FREE(q);
		}
		bsdconv_hash_delete(ins, HASHKEY);
	}
}

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=ins->phase[ins->phase_index].codec[this_phase->index].priv;
	struct data_rt *q;
	data=this_phase->curr->data;
	int i;
	int ucs=0;

	this_phase->state.status=NEXTPHASE;

	if(data[0]==0x1){
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
			DATA_MALLOC(q->data);
			if(t->dlast!=NULL)
				*(t->dlast)=q->data;
			t->dlast=&(((struct data_rt *)q->data)->next);
			*((struct data_rt *)q->data)=*(this_phase->curr);
			((struct data_rt *)q->data)->next=NULL;
			this_phase->curr->flags &= ~F_FREE;
			q->len=t->offsetA;

			t->rerail->match=t->queue->data;
			t->rerail->pend=1;

			return;
		}
	}
	t->offsetA+=1;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	return;
}