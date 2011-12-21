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
	char f;
};

void cbcreate(struct bsdconv_instance *ins){
	CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->p=NULL;
	r->q=NULL;
	r->r=&(r->q);
	r->f=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	free(CURRENT_CODEC(ins)->priv);
}

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	data=this_phase->curr->data;

	if(r->f==0){
		if(data[0]==0x3 && data[1]>0x7f){
			r->f=1;
			DATA_MALLOC(r->p);
			*(r->p)=*(this_phase->curr);
			this_phase->curr->flags &= ~F_FREE;
			this_phase->state.status=CONTINUE;
			return;
		}else{
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			*(this_phase->data_tail)=*(this_phase->curr);
			this_phase->curr->flags &= ~F_FREE;
			this_phase->state.status=NEXTPHASE;
			return;
		}
	}else if(r->f){
		if(data[0]==0x1b){
			DATA_MALLOC(*(r->r));
			**(r->r)=*(this_phase->curr);
			this_phase->curr->flags &= ~F_FREE;
			r->r=&((*(r->r))->next);

			this_phase->state.status=CONTINUE;
			return;
		}else{
			r->f=0;

			this_phase->data_tail->next=r->p;
			this_phase->data_tail=this_phase->data_tail->next;

			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			*(this_phase->data_tail)=*(this_phase->curr);
			this_phase->curr->flags &= ~F_FREE;

			if(r->q){
				this_phase->data_tail->next=r->q;
				*(r->r)=NULL;
				while(this_phase->data_tail->next){
					this_phase->data_tail=this_phase->data_tail->next;
				}
			}
			r->p=r->q=NULL;
			r->r=&(r->q);
			r->f=0;
			this_phase->state.status=NEXTPHASE;
			return;
		}
	}
}
