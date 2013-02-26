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

void cbcreate(struct bsdconv_instance *ins){
	struct my_s *r=malloc(sizeof(struct my_s));
	r->data.data=NULL;
	CURRENT_CODEC(ins)->priv=r;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	CURRENT_CODEC(ins)->data_z=0;
	t->data.len=0;
	if(t->data.data)
		free(t->data.data);
	t->data.next=0;
	t->flag=F_CLEAR;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	if(t->data.data){
		free(t->data.data);
	}
	free(t);
}

int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void cbconv(struct bsdconv_instance *ins){
	struct data_rt *data_ptr;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
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
		this_phase->state.data=&(t->data);
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
