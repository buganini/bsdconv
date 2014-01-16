/*
 * Copyright (c) 2011-2014 Kuan-Chung Chiu <buganini@gmail.com>
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct bsdconv_filter *filter;
	int limit;
	int passed;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->filter=NULL;
	r->limit=0;
	while(arg){
		if(strcasecmp(arg->key, "FOR")==0){
			r->filter=load_filter(arg->ptr);
			if(r->filter==NULL){
				free(r);
				return EOPNOTSUPP;
			}
		}else if(strcasecmp(arg->key, "LIMIT")==0){
			sscanf(arg->ptr, "%d", &r->limit);
		}else{
			free(r);
			return EINVAL;
		}
		arg=arg->next;
	}
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->passed=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	int pass=1;

	if(t->filter!=NULL && !t->filter->cbfilter(this_phase->curr)){
		pass=0;
	}

	if(pass && t->limit!=0){
		if(t->passed < t->limit){
			t->passed += 1;
		}else{
			pass=0;
		}
	}

	if(pass){
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		*(this_phase->data_tail)=*(this_phase->curr);
		this_phase->data_tail->next=NULL;
		this_phase->curr->flags &= ~F_FREE;

		this_phase->i=this_phase->curr->len-1;
		this_phase->state.status=NEXTPHASE;
	}else{
		this_phase->state.status=DEADEND;
	}

	return;
}
