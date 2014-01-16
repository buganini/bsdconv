/*
 * Copyright (c) 2009-2014 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <errno.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_filter *filter;
	bsdconv_counter_t *counter;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->filter=NULL;

	char *key="COUNT";
	while(arg){
		if(strcasecmp(arg->key, "FOR")==0){
			r->filter=load_filter(arg->ptr);
			if(r->filter==NULL){
				free(r);
				return EOPNOTSUPP;
			}
		}else{
			key=arg->key;
		}
		arg=arg->next;
	}
	r->counter=bsdconv_counter(ins, key);
	return 0;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;

	if(r->filter==NULL || r->filter->cbfilter(this_phase->curr))
		*(r->counter)+=1;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	this_phase->state.status=NEXTPHASE;
	return;
}
