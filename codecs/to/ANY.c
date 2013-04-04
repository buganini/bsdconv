/*
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

#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

int cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	struct data_st *r=malloc(sizeof(struct data_st));
	if(arg){
		str2data(arg->key, r);
	}else{
		r->len=2;
		r->data=strdup("\x01\x3f");
	}
	CURRENT_CODEC(ins)->priv=r;
	return 1;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct data_st *r=this_phase->codec[this_phase->index].priv;
	free(r->data);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct data_st *d=this_phase->codec[this_phase->index].priv;
	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->len=d->len;
	this_phase->data_tail->flags=0;
	this_phase->data_tail->data=d->data;

	this_phase->state.status=NEXTPHASE;

	ins->oerr+=1;
	return;
}
