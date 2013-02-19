/*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
};

void cbcreate(struct bsdconv_instance *ins){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->status=0;
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;

	this_phase->pend=0;
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
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phase_index-1];
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	struct data_st data;
	struct data_rt *data_ptr;

	memcpy(&data, (char *)(this_phase->codec[this_phase->index].data_z+(uintptr_t)this_phase->state.data), sizeof(struct data_st));
	data.data=this_phase->codec[this_phase->index].data_z+(uintptr_t)data.data;

	if(data.len>1 && t->status==0){
		t->status=1;
		this_phase->pend=1;

		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=1;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(1);
		*CP(this_phase->data_tail->data)='\x0E';
	}else if(data.len==1 && t->status!=0){
		t->status=0;
		this_phase->pend=0;

		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=1;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(1);
		*CP(this_phase->data_tail->data)='\x0F';
	}

	if(t->status){
		LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

		this_phase->data_head->len=0;
		this_phase->match=1;
		this_phase->match_data=NULL;
		this_phase->pend=1;

		this_phase->bak=this_phase->curr->next;
		LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
		this_phase->curr=prev_phase->data_head;

		RESET(ins->phase_index);

		ins->phase_index+=1;

		this_phase->state.status=NOOP;
	}else{
		LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

		this_phase->state.status=NEXTPHASE;
	}
}
