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
#include <sys/types.h>
#include "../../src/bsdconv.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

struct my_s{
	int status;
	int *tbl;
	int b;
	char buf[4];
	char unicode;
};

void *cbcreate(void){
	return malloc(sizeof(struct my_s));
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->status=0;
	r->unicode=0;
}

void cbdestroy(void *p){
	free(p);
}

#define CONTINUE() do{	\
	this_phase->state.status=CONTINUE;	\
	return;	\
}while(0);

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->status=0;	\
	return;	\
}while(0);

int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void callback(struct bsdconv_instance *ins){
	char *p;
	int i,j;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	char d=CP(this_phase->data->data)[this_phase->i];

	switch(t->status){
		case 0:
			if(d=='u'){
				t->status=40;
				CONTINUE();
			}else if(hex[(unsigned char)d]==-1){
				DEADEND();
			}else{
				t->status=21;
				t->buf[0]=hex[(unsigned char)d];
				CONTINUE();
			}
			break;
		case 21:
			if(hex[(unsigned char)d]==-1){
				DEADEND();
			}else{
				t->buf[0]*=16;
				t->buf[0]+=hex[(unsigned char)d];
				DATA_MALLOC(this_phase->data_tail->next);
				this_phase->data_tail=this_phase->data_tail->next;
				this_phase->data_tail->next=NULL;
				this_phase->data_tail->flags=F_FREE;
				this_phase->data_tail->len=2;
				p=this_phase->data_tail->data=malloc(2);
				p[0]=0x03;
				p[1]=t->buf[0];
				this_phase->state.status=NEXTPHASE;
				t->status=0;
				return;				
			}
			break;
		case 40:
			if(hex[(unsigned char)d]==-1){
				DEADEND();
			}else{
				t->status=41;
				t->buf[0]=hex[(unsigned char)d];
				CONTINUE();
			}
			break;
		case 41:
			if(hex[(unsigned char)d]==-1){
				DEADEND();
			}else{
				t->status=42;
				t->buf[0]*=16;
				t->buf[0]+=hex[(unsigned char)d];
				CONTINUE();
			}
			break;
		case 42:
			if(hex[(unsigned char)d]==-1){
				DEADEND();
			}else{
				t->status=43;
				t->buf[1]=hex[(unsigned char)d];
				CONTINUE();
			}
			break;
		case 43:
			if(hex[(unsigned char)d]==-1){
				DEADEND();
			}else{
				i=0;
				t->buf[1]*=16;
				t->buf[1]+=hex[(unsigned char)d];
				while(t->buf[i]==0)++i;
				DATA_MALLOC(this_phase->data_tail->next);
				this_phase->data_tail=this_phase->data_tail->next;
				this_phase->data_tail->next=NULL;
				this_phase->data_tail->flags=F_FREE;
				this_phase->data_tail->len=3-i;
				p=this_phase->data_tail->data=malloc(3-i);
				p[0]=0x01;
				for(j=1;i<2;++i,++j){
					p[j]=t->buf[i];
				}
				this_phase->state.status=NEXTPHASE;
				t->status=0;
				return;				
			}
			break;
	}
}
