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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
	char plane, buf[4];
	struct bsdconv_codec_t cd;
};

void *cbcreate(void){
	struct my_s *r=malloc(sizeof(struct my_s));
	if(!loadcodec(&r->cd, "inter/UNICODE", 1)){
		free(r);
		return NULL;
	}
	return r;
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->status=0;
	r->plane=1;
}

void cbdestroy(void *p){
	struct my_s *r=p;
	unloadcodec(&r->cd);
	free(p);
}

#define CONTINUE() do{	\
	this_phase->state.status=CONTINUE;	\
	return;	\
}while(0);

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	char d=this_phase->data->data[this_phase->i], *p;
	struct state_rt state;
	struct data_rt *data_ptr;

	int i;
	switch(t->status){
		case 0:
			if(d==0x0){ //plane switch sequence
				t->status=10;
				CONTINUE();
			}else{ //data sequence
				t->status=1;
				t->buf[0]=0x02;
				t->buf[1]=t->plane;
				t->buf[2]=d;
				CONTINUE();
			}
			break;
		case 1:
			t->status=0;
			t->buf[3]=d;
			memcpy(&state, t->cd.z, sizeof(struct state_st));
			for(i=0;i<4;++i){
				memcpy(&state, t->cd.z + (uintptr_t)state.sub[(unsigned char)t->buf[i]], sizeof(struct state_st));
				if(state.status==DEADEND){
					break;
				}
			}
			this_phase->state.status=NEXTPHASE;
			switch(state.status){
				case MATCH:
				case SUBMATCH:
					LISTCPY(this_phase->data_tail, state.data, t->cd.z);
					return;
				default:
					this_phase->data_tail->next=malloc(sizeof(struct data_st));
					this_phase->data_tail=this_phase->data_tail->next;
					this_phase->data_tail->next=NULL;
					this_phase->data_tail->len=4;
					this_phase->data_tail->setmefree=1;
					p=this_phase->data_tail->data=malloc(4);
					memcpy(p,t->buf,4);
					return;
			}
		case 10:
			t->status=0;
			t->plane=d;
			CONTINUE();
			break;
	}
}
