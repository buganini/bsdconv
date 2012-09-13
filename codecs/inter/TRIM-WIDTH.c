/*
 * Reference: http://blog.oasisfeng.com/2006/10/19/full-cjk-unicode-range/
 * Some code come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 *
 * Copyright (c) 2012 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_instance *ins;
	struct data_rt *q, *qt;
	char ambi_width;
	size_t width;
	size_t remain;
};

void cbcreate(struct bsdconv_instance *ins){
	char *p;
	struct my_s *r=CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));	

	r->ambi_width=1;

	p=getenv("BSDCONV_SET_WIDE_AMBI");
	if(p){
		r->ambi_width=2;
	}

	p=getenv("BSDCONV_TRIM_WIDTH");
	if(p){
		int t;
		sscanf(p,"%d",&t);
		r->width=t;
	}

	r->ins=bsdconv_create("WIDTH");
	r->q=malloc(sizeof(struct data_rt));
	r->q->next=NULL;
}

void cbinit(struct bsdconv_instance *inst){
	struct my_s *r=CURRENT_CODEC(inst)->priv;
	struct bsdconv_instance *ins=r->ins;
	struct data_rt *t;
	bsdconv_init(ins);
	while(r->q->next){
		t=r->q->next->next;
		DATA_FREE(r->q->next);
		r->q->next=t;
	}
	r->qt=r->q;
	r->remain=r->width;
}

void cbctl(struct bsdconv_instance *ins, int ctl, void *ptr, size_t v){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	switch(ctl){
		case BSDCONV_SET_WIDE_AMBI:
			r->ambi_width=2;
			break;
		case BSDCONV_SET_TRIM_WIDTH:
			r->width=v;
			break;
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	struct data_rt *t;
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	bsdconv_destroy(r->ins);
	while(r->q->next){
		t=r->q->next->next;
		DATA_FREE(r->q->next);
		r->q->next=t;
	}
	free(r->q);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;

	bsdconv_init(r->ins);
	r->ins->input=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	r->ins->input.next=NULL;
	r->ins->flush=1;
	bsdconv(r->ins);

	int w=r->ins->full*2 + r->ins->half + r->ins->ambi*r->ambi_width;
	if(r->remain >= w){
		r->qt->next=r->ins->phase[r->ins->phasen].data_head->next;
		r->qt=r->qt->next;
		r->ins->phase[r->ins->phasen].data_head->next=NULL;
		r->remain -= w;
	}else{
		r->remain=0;
		if(r->q->next){
			this_phase->data_tail->next=r->q->next;
			r->q->next=NULL;
			r->qt=r->q;
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
