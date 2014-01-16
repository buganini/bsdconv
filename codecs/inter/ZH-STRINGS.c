/*
 * Copyright (c) 2012-2014 Kuan-Chung Chiu <buganini@gmail.com>
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
#include "../filter/cjk.c"
#include <stdlib.h>
#include <string.h>

struct my_s {
	struct data_rt *qh, *qt;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	DATA_MALLOC(r->qh);
	r->qh->flags=0;
	r->qh->next=NULL;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct data_rt *t;
	while(r->qh->next){
		t=r->qh->next->next;
		DATUM_FREE(r->qh->next);
		r->qh->next=t;
	}
	r->qt=r->qh;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct data_rt *t;
	while(r->qh){
		t=r->qh->next;
		DATUM_FREE(r->qh);
		r->qh=t;
	}
	free(r);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;

	if(r->qh->next){
		DATA_MALLOC(r->qt->next);
		r->qt=r->qt->next;
		r->qt->data="\x01\n";
		r->qt->len=2;
		r->qt->flags=0;
		r->qt->next=NULL;

		this_phase->data_tail->next=r->qh->next;
		this_phase->data_tail=r->qt;
		r->qh->next=NULL;
		r->qt=r->qh;
	}

	this_phase->state.status=NEXTPHASE;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;

	if(cbfilter(this_phase->curr)){
		DATA_MALLOC(r->qt->next);
		r->qt=r->qt->next;
		*(r->qt)=*(this_phase->curr);
		this_phase->curr->flags &= ~F_FREE;
		r->qt->next=NULL;

		this_phase->state.status=SUBMATCH;
		return;
	}

	cbflush(ins);
}
