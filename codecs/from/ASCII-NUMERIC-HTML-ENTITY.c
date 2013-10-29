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

#define USE_HEX_MAP
#define USE_DEC_MAP

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
	int *tbl;
	int b;
	union {
		char c[4];
		uint32_t i;
	} buf;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->status=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	void *p=CURRENT_CODEC(ins)->priv;
	free(p);
}

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->status=0;	\
	return;	\
}while(0);

void cbconv(struct bsdconv_instance *ins){
	char ob[8], *p;
	int i,j=0;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	char d;

	for(;this_phase->i<this_phase->curr->len;this_phase->i+=1){
		d=CP(this_phase->curr->data)[this_phase->i];
		if(d==';' && t->status){
			//put data
			t->buf.i=htobe32(t->buf.i);
			for(i=0;i<4;i++){
				if(t->buf.c[i] || j)
					ob[j++]=t->buf.c[i];
			}
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=j+1;
			p=this_phase->data_tail->data=malloc(j+1);
			p[0]=0x01;
			memcpy(&p[1], ob, j);
			this_phase->state.status=NEXTPHASE;
			t->status=0;
			return;
		}
		if(t->status){
			++t->status;
			if(t->tbl[(unsigned char)d]==-1) DEADEND();
			t->buf.i*=t->b;
			t->buf.i+=t->tbl[(unsigned char)d];
		}else{
			if(d=='x'){
				t->status=1000;
				t->tbl=hex;
				t->b=16;
				t->buf.i=0;
				continue;
			}
			t->b=10;
			t->tbl=dec;
			if(t->tbl[(unsigned char)d]==-1) DEADEND();
			t->buf.i=t->tbl[(unsigned char)d];
			t->status=1;
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
