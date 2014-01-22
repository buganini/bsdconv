/*
 * Copyright (c) 2013 Kuan-Chung Chiu <buganini@gmail.com>
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
	struct data_rt *from;
	struct data_rt *to;
	struct data_rt *cursor;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	int e;
	r->from=NULL;
	r->to=NULL;
	while(arg){
		DATA_FREE(r->from);
		DATA_FREE(r->to);
		r->from=str2data(arg->key, &e, ins);
		if(e){
			free(r);
			return e;
		}
		if(arg->ptr){
			r->to=str2data(arg->ptr, &e, ins);
			if(e){
				DATA_FREE(r->from);
				free(r);
				return e;
			}
		}
		arg=arg->next;
	}
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	DATA_FREE(r->from);
	DATA_FREE(r->to);
	free(r);
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->cursor=r->from;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;
	unsigned char *datai=this_phase->curr->data;
	unsigned char *datar=r->cursor->data;
	size_t l=this_phase->curr->len;
	size_t i;

	if(l != r->cursor->len){
		this_phase->state.status=DEADEND;
		return;
	}

	for(i=0;i<l;i+=1){
		if(datai[i] != datar[i]){
			this_phase->state.status=DEADEND;
			return;
		}
	}

	if(r->cursor->next != NULL){
		r->cursor = r->cursor->next;
		this_phase->state.status=CONTINUE;
		return;
	}else{
		r->cursor = r->from;
		LISTCPY(this_phase->data_tail, r->to);
		this_phase->state.status=NEXTPHASE;
		return;
	}
}
