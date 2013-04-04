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
	char *buf;
	char *p, f;
};

int cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	r->buf=malloc(8);
	CURRENT_CODEC(ins)->priv=r;
	return 1;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->status=0;
	r->p=r->buf+1;
	r->f=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r->buf);
	free(r);
}

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->status=0;	\
	t->p=t->buf+1;	\
	t->f=0;	\
	return;	\
}while(0);

#define PASS() do{	\
	DATA_MALLOC(this_phase->data_tail->next);	\
	this_phase->data_tail=this_phase->data_tail->next;	\
	this_phase->data_tail->next=NULL;	\
	this_phase->data_tail->len=t->p - t->buf;	\
	this_phase->data_tail->flags=F_FREE;	\
	this_phase->state.status=NEXTPHASE;	\
	this_phase->data_tail->data=t->buf;	\
	t->buf[0]=0x01;	\
	t->status=0;	\
	t->f=0;	\
	t->buf=malloc(8);	\
	t->p=t->buf+1;	\
	return;	\
}while(0);

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	char d;

	for(;this_phase->i<this_phase->curr->len;this_phase->i+=1){
		d=CP(this_phase->curr->data)[this_phase->i];
		switch(t->status){
			case 0:
				if((d & bb10000000) == 0){
					/* exclude ASCII */
					DEADEND();

					/* Unreachable */
					*(t->p)=d;
					if(*(t->p)){ t->p+=1; }
					PASS();
				}else if((d & bb11100000) == bb11000000){
					t->status=21;
					*(t->p)=(d >> 2) & bb00000111;
					if(*(t->p)){ t->p+=1; t->f=1;}
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
					if(t->f || *(t->p)){ t->p+=1; }
					PASS();
				}else{
					DEADEND();
				}
				break;
			case 31:
				if((d & bb11000000) == bb10000000){
					t->status=32;
					*(t->p) |= (d >> 2) & bb00001111;
					if(t->f || *(t->p)){ t->p+=1; t->f=1;}
					*(t->p)=(d << 6) & bb11000000;
					continue;
				}else{
					DEADEND();
				}
				break;
			case 32:
				if((d & bb11000000) == bb10000000){
					*(t->p) |= d & bb00111111;
					if(t->f || *(t->p)){ t->p+=1; }
					PASS();
				}else{
					DEADEND();
				}
				break;
			case 41:
				if((d & bb11000000) == bb10000000){
					t->status=42;
					*(t->p) |= (d >> 4) & bb00000011;
					if(t->f || *(t->p)){ t->p+=1; t->f=1;}
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
					if(t->f || *(t->p)){ t->p+=1; t->f=1;}
					*(t->p)=(d << 6) & bb11000000;
					continue;
				}else{
					DEADEND();
				}
				break;
			case 43:
				if((d & bb11000000) == bb10000000){
					*(t->p) |= d & bb00111111;
					if(t->f || *(t->p)){ t->p+=1; }
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

#if 0
//This is a simpler version, but somehow slower.
#include <stdlib.h>
#include "../../src/bsdconv.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

struct my_s{
	unsigned char w;
	union {
		unsigned char bytes[4];
		uint32_t ucs4;
	} ucs4;
};

int cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
}

void cbinit(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	t->w=0;
	t->ucs4.ucs4=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r);
}

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->w=0;	\
	return;	\
}while(0);

#define PASS() do{	\
	t->ucs4.ucs4=htonl(t->ucs4.ucs4);	\
	for(i=0;t->ucs4.bytes[i]==0 && i<4;++i);	\
	DATA_MALLOC(this_phase->data_tail->next);	\
	this_phase->data_tail=this_phase->data_tail->next;	\
	this_phase->data_tail->next=NULL;	\
	this_phase->data_tail->len=5 - i;	\
	this_phase->data_tail->data=c=malloc(5 - i);	\
	this_phase->data_tail->flags=F_FREE;	\
	this_phase->state.status=NEXTPHASE;	\
	*c=0x01;	\
	c+=1;	\
	for(;i<4;++i,c+=1){	\
		*c=t->ucs4.bytes[i];	\
	}	\
	return;	\
}while(0);

void cbconv(struct bsdconv_instance *ins){
	int i;
	unsigned char *c;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	char d;

	for(;this_phase->i<this_phase->curr->len;this_phase->i+=1){
		d=CP(this_phase->curr->data)[this_phase->i];
		switch(t->w){
			case 0:
				if((d & bb10000000) == 0){
					/* exclude ASCII */
					DEADEND();

					/* Unreachable */
					t->ucs4.ucs4=d & bb01111111;
					PASS();
				}else if((d & bb11100000) == bb11000000){
					t->w=1;
					t->ucs4.ucs4=d & bb00011111;
				}else if((d & bb11110000) == bb11100000){
					t->w=2;
					t->ucs4.ucs4=d & bb00001111;
				}else if((d & bb11111000) == bb11110000){
					t->w=3;
					t->ucs4.ucs4=d & bb00000111;
				}else{
					DEADEND();
				}
				continue;
			case 1:
			case 2:
			case 3:
				if((d & bb11000000) != bb10000000){
					DEADEND();
				}
				t->ucs4.ucs4 <<= 6;	
				t->w-=1;
				t->ucs4.ucs4 |= d & bb00111111;
				if(t->w==0){
					PASS();
				}
				continue;
			default:
				DEADEND();
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
#endif	
