/*
 * Copyright (c) 2009,2010 Kuan-Chung Chiu <buganini@gmail.com>
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
	struct data_s data;
	/* extend struct data_s */
	size_t size;
	unsigned char flag;
};

void *cbcreate(void){
	struct my_s *r=malloc(sizeof(struct my_s));
	r->data.data=NULL;
	return r;
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *t){
	cdc->data_z=0;
	t->data.len=0;
	t->data.data=NULL;
	t->data.next=0;
	t->size=0;
	t->flag=F_A;
}

void cbdestroy(void *p){
	struct my_s *t=p;
	if(t->data.data){
		free(t->data.data);
	}
	free(p);
}

int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void callback(struct bsdconv_instance *ins){
	void *p;
	struct bsdconv_phase *this_phase=&ins->phase[0];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	unsigned char d=*ins->from_data;
	if(hex[d]==-1){
		this_phase->state.status=DEADEND;
		t->flag=F_CLEAR;
	}else{
		if(t->flag==F_CLEAR){
			t->flag=F_A;
			t->data.len=0;
		}

		if(t->data.len)
			this_phase->state.status=SUBMATCH;
		else
			this_phase->state.status=CONTINUE;
		this_phase->state.data=&(t->data);
		switch(t->flag){
			case F_A:
				if(t->data.len >= t->size){
					t->size+=8;
					p=t->data.data;
					t->data.data=realloc(t->data.data,t->size);
					if(p!=t->data.data){
						free(p);
					}
				}
				t->data.data[t->data.len]=hex[d];
				t->data.len+=1;
				t->flag=F_B;
				break;
			case F_B:
				t->data.data[t->data.len-1]<<=4;
				t->data.data[t->data.len-1]|=hex[d];
				t->flag=F_A;
				break;
		}
	}
}
