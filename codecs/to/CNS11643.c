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
	struct bsdconv_codec_t cd;
};

void *cbcreate(void){
	struct my_s *r=malloc(sizeof(struct my_s));
	if(!loadcodec(&r->cd, "inter/CNS11643", 1)){
		free(r);
		return NULL;
	}
	return r;
}

void cbdestroy(void *p){
	struct my_s *r=p;
	unloadcodec(&r->cd);
	free(p);
}

#define bb10000000 0x80

void callback(struct bsdconv_instance *ins){
	char *data;
	unsigned int len;
	struct state_s state;
	struct data_s *data_ptr;
	char *ptr;
	int i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	data=ins->phase[ins->phasen-1].data->data;

	if(ins->phase[ins->phasen-1].data->len==2 && (data[1] & bb10000000)==0){
		this_phase->state.status=DEADEND;
		return;
	}
	switch(*data){
		case 0x01:
			memcpy(&state, t->cd.z, sizeof(struct state_s));
			for(i=0;i<ins->phase[ins->phasen-1].data->len;++i){
				memcpy(&state, t->cd.z + (uintptr_t)state.sub[(unsigned char)data[i]], sizeof(struct state_s));
				if(state.status==DEADEND){
					break;
				}
			}
			switch(state.status){
				case MATCH:
				case SUBMATCH:
					this_phase->state.status=NEXTPHASE;
					for(data_ptr=state.data;data_ptr;){
						this_phase->data_tail->next=malloc(sizeof(struct data_s));
						this_phase->data_tail=this_phase->data_tail->next;
						memcpy(this_phase->data_tail, t->cd.z+(uintptr_t)data_ptr, sizeof(struct data_s));
						data_ptr=this_phase->data_tail->next;
						this_phase->data_tail->next=NULL;
						ptr=t->cd.z+(uintptr_t)this_phase->data_tail->data;
						this_phase->data_tail->data=malloc(this_phase->data_tail->len);
						memcpy(this_phase->data_tail->data, ptr, this_phase->data_tail->len);
						this_phase->data_tail->data[0]=0;
					}
					return;
				default:
					this_phase->state.status=DEADEND;
					return;
			}
		case 0x02:
			len=ins->phase[ins->phasen-1].data->len-1;

			this_phase->data_tail->next=malloc(sizeof(struct data_s));
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			
			this_phase->data_tail->len=4;
			this_phase->data_tail->data=malloc(4);
			memcpy(this_phase->data_tail->data, data, this_phase->data_tail->len);
			this_phase->data_tail->data[0]=0;
			this_phase->state.status=NEXTPHASE;
			return;
		default:
			this_phase->state.status=DEADEND;
			return;
	}
}
