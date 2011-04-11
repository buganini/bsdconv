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
#include "../../src/bsdconv.h"

struct my_s{
	int status;
	char buf[16];
	char *p;
};

void *cbcreate(void){
	return  malloc(sizeof(struct my_s));
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->status=0;
	r->p=r->buf;
}

void cbdestroy(void *p){
	free(p);
}

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->status=0;	\
	t->p=t->buf;	\
	return;	\
}while(0);

#define PASS() do{	\
	this_phase->state.status=NEXTPHASE;	\
	t->status=0;	\
	t->p=t->buf;	\
	return;	\
}while(0);

#define APPEND(n) do{	\
	DATA_MALLOC(this_phase->data_tail->next);	\
	this_phase->data_tail=this_phase->data_tail->next;	\
	this_phase->data_tail->next=NULL;	\
	this_phase->data_tail->len=n+1;	\
	this_phase->data_tail->flags=F_FREE;	\
	p=this_phase->data_tail->data=malloc(n+1);	\
	p[0]=0x01;	\
}while(0);

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	char d, *p;
	int n,i;

	for(;this_phase->i<this_phase->data->len;this_phase->i+=1){
		d=CP(this_phase->data->data)[this_phase->i];
		switch(t->status){
			case 0:
				if((d & bb10000000) == 0){
					/* exclude ASCII */
					DEADEND();
				}else if((d & bb11100000) == bb11000000){
					t->status=21;
					*(t->p)=(d >> 2) & bb00000111;
					if(*(t->p)) t->p+=1;
					*(t->p)=(d << 6) & bb11000000;
				}else if((d & bb11110000) == bb11100000){
					t->status=31;
					*(t->p)=(d << 4) & bb11110000;
				}else if((d & bb11111000) == bb11110000){
					t->status=41;
					*(t->p)=(d << 2) & bb00011100;
				}else{
					DEADEND();
				}
				continue;
				break;
			case 21:
				if((d & bb11000000) == bb10000000){
					*(t->p) |= d & bb00111111;
					if(*(t->p)) t->p+=1;
					n=t->p - t->buf;
					APPEND(n);
					for(i=0;i<n;++i)
						p[i+1]=t->buf[i];
					PASS();
				}else{
					DEADEND();
				}
				break;
			case 31:
				if((d & bb11000000) == bb10000000){
					t->status=32;
					*(t->p) |= (d >> 2) & bb00001111;
					if(*(t->p)) t->p+=1;
					*(t->p)=(d << 6) & bb11000000;
					continue;
				}else{
					DEADEND();
				}
				break;
			case 32:
				if((d & bb11000000) == bb10000000){
					*(t->p) |= d & bb00111111;
					if(*(t->p)) t->p+=1;
					n=t->p - t->buf;
					APPEND(n);
					for(i=0;i<n;++i)
						p[i+1]=t->buf[i];
					PASS();
				}else{
					DEADEND();
				}
				break;
			case 41:
				if((d & bb11000000) == bb10000000){
					t->status=42;
					*(t->p) |= (d >> 4) & bb00000011;
					if(*(t->p)) t->p+=1;
					*(t->p)=(d << 4) & bb11110000;
					continue;
				}else{
					DEADEND();
				}
				break;
			case 42:
				if((d & bb11000000) == bb10000000){
					t->status=43;
					*(t->p) |= (d >> 2) & bb00001111;
					if(*(t->p)) t->p+=1;
					*(t->p)=(d << 6) & bb11000000;
					continue;
				}else{
					DEADEND();
				}
				break;
			case 43:
				if((d & bb11000000) == bb10000000){
					*(t->p) |= d & bb00111111;
					if(*(t->p)) t->p+=1;
					n=t->p - t->buf;
					APPEND(n);
					for(i=0;i<n;++i)
						p[i+1]=t->buf[i];
					PASS();
				}else{
					DEADEND();
				}
				break;
			default:
				DEADEND();
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
