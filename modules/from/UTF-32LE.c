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


struct my_s{
	int status;
	char buf[4];
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->status=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *p=THIS_CODEC(ins)->priv;
	free(p);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	char d;
	int i;
	size_t l;

	for(;this_phase->i<this_phase->curr->len;this_phase->i+=1){
		d=CP(this_phase->curr->data)[this_phase->i];
		switch(t->status){
			case 0:
				t->buf[3]=d;
				t->status=1;
				continue;
				break;
			case 1:
				t->buf[2]=d;
				t->status=2;
				continue;
				break;
			case 2:
				t->buf[1]=d;
				t->status=3;
				continue;
				break;
			case 3:
				t->buf[0]=d;
				t->status=0;
				for(i=0;i<4;++i){
					if(t->buf[i]) break;
				}
				l=(4-i)+1;
				DATA_MALLOC(this_phase->data_tail->next);
				this_phase->data_tail=this_phase->data_tail->next;
				this_phase->data_tail->next=NULL;
				this_phase->data_tail->len=l;
				this_phase->data_tail->flags=F_FREE;
				this_phase->data_tail->data=malloc(l);
				CP(this_phase->data_tail->data)[0]=0x01;
				memcpy(CP(this_phase->data_tail->data)+1, &t->buf[i], l-1);
				this_phase->state.status=NEXTPHASE;
				return;
				break;
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
