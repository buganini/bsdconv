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
	char *buf;
	char *p,f;
};

void *cbcreate(void){
	struct my_s *r=malloc(sizeof(struct my_s));
	r->buf=malloc(32);
	return r;
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->p=r->buf;
	r->f=0;
}

void cbdestroy(struct my_s *r){
	free(r->buf);
	free(r);
}

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	char d=CP(this_phase->data->data)[this_phase->i];

	if(t->f){
		*(t->p)=d;
		t->p+=1;
		if(d==';' || (t->p - t->buf)==30){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->len=t->p - t->buf;
			this_phase->data_tail->flags=F_FREE;
			this_phase->state.status=NEXTPHASE;
			this_phase->data_tail->data=t->buf;
			t->f=0;
			t->buf=malloc(32);
			t->p=t->buf;
		}
	}else if(d==0x1b){
		t->f=1;
		*(t->p)=d;
		t->p+=1;
		this_phase->state.status=CONTINUE;
	}
}
