/*
 * Some code and table come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
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

#include <errno.h>
#include  <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct data_rt *after;
	struct data_rt *before;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	int e;
	r->after=NULL;
	r->before=NULL;
	while(arg){
		if(strcmp(arg->key, "AFTER")==0){
			if(r->after)
				DATA_FREE(r->after);
			r->after=str2data(arg->ptr, &e, ins);
			if(e){
				if(r->after)
					DATA_FREE(r->after);
				free(r);
				return e;
			}
		}else if(strcmp(arg->key, "BEFORE")==0){
			if(r->before)
				DATA_FREE(r->before);
			r->before=str2data(arg->ptr, &e, ins);
			if(e){
				if(r->before)
					DATA_FREE(r->before);
				free(r);
				return e;
			}
		}else{
			return EINVAL;
		}
		arg=arg->next;
	}
	CURRENT_CODEC(ins)->priv=r;
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	if(r->after)
		DATA_FREE(r->after);
	if(r->before)
		DATA_FREE(r->before);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;

	if(r->before)
		LISTCPY(this_phase->data_tail, r->before);

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	if(r->after)
		LISTCPY(this_phase->data_tail, r->after);

	this_phase->state.status=NEXTPHASE;
	return;
}
