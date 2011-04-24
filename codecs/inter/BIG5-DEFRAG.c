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

#include "../../src/bsdconv.h"

struct my_s{
	struct data_rt *p;
	struct data_rt *q;
	struct data_rt **r;
};

void *cbcreate(void){
	return malloc(sizeof(struct my_s));
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->p=NULL;
	r->q=NULL;
	r->r=&(r->q);
}

void cbdestroy(void *r){
	free(r);
}

void callback(struct bsdconv_instance *ins){
	char *data;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *r=this_phase->codec[this_phase->index].priv;
	data=this_phase->data->data;

	if(r->p==NULL){
		if(data[0]==0x3){
			DATA_MALLOC(r->p);
			*(r->p)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			this_phase->state.status=CONTINUE;
			return;
		}else{
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			*(this_phase->data_tail)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			this_phase->data_tail->next=NULL;
			this_phase->state.status=NEXTPHASE;
			r->p=r->q=NULL;
			r->r=&(r->q);
			return;
		}
	}else if(r->q==NULL){
		if(data[0]==0x1b){
			DATA_MALLOC(*(r->r));
			**(r->r)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			r->r=&((*(r->r))->next);
			this_phase->state.status=CONTINUE;
			return;
		}else{
			this_phase->data_tail->next=r->p;
			this_phase->data_tail=this_phase->data_tail->next;
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			*(this_phase->data_tail)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			this_phase->data_tail->next=NULL;
			this_phase->state.status=NEXTPHASE;
			r->p=r->q=NULL;
			r->r=&(r->q);
			return;
		}
	}else{
		if(data[0]==0x1b){
			DATA_MALLOC(*(r->r));
			**(r->r)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			r->r=&((*(r->r))->next);
			this_phase->state.status=CONTINUE;
			return;
		}else if(data[0]==0x3){
			this_phase->data_tail->next=r->p;
			this_phase->data_tail=this_phase->data_tail->next;
			DATA_MALLOC(this_phase->data_tail->next);
			*(this_phase->data_tail->next)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=r->q;
			*(r->r)=NULL;
			this_phase->state.status=NEXTPHASE;
			r->p=r->q=NULL;
			r->r=&(r->q);
			return;
		}else{
			this_phase->data_tail->next=r->p;
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=r->q;
			DATA_MALLOC(*(r->r));
			this_phase->data_tail=*(r->r);
			*(this_phase->data_tail)=*(this_phase->data);
			this_phase->data->flags &= ~F_FREE;
			this_phase->data_tail->next=NULL;
			this_phase->state.status=NEXTPHASE;
			r->p=r->q=NULL;
			r->r=&(r->q);
			return;
		}
	}
}
