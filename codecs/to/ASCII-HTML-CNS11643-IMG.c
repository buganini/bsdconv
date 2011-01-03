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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void *cbcreate(void){
	struct bsdconv_codec_t *cd=malloc(sizeof(struct bsdconv_codec_t));
	if(!loadcodec(cd, "inter/CNS11643", 1)){
		free(cd);
		return NULL;
	}
	return cd;
}

void cbdestroy(void *p){
	struct bsdconv_codec_t *cd=p;
	unloadcodec(cd);
	free(p);
}

void callback(struct bsdconv_instance *ins){
	char *data, *p, buf[128]={0};
	unsigned int len, i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	data=this_phase->data->data;
	struct state_st state;
	struct data_rt *data_ptr, *orig_next, *my_tail;
	struct bsdconv_codec_t *t=this_phase->codec[this_phase->index].priv;
	switch(*data){
		case 0x01:
			memcpy(&state, t->z, sizeof(struct state_st));
			for(i=0;i<this_phase->data->len;++i){
				memcpy(&state, t->z + (uintptr_t)state.sub[(unsigned char)data[i]], sizeof(struct state_st));
				if(state.status==DEADEND){
					break;
				}
			}
			switch(state.status){
				case MATCH:
				case SUBMATCH:
					orig_next=this_phase->data->next;
					free(data);
					my_tail=this_phase->data;
					memcpy(my_tail, t->z+(uintptr_t)state.data, sizeof(struct data_st));
					my_tail->data=t->z+(uintptr_t)my_tail->data;
					my_tail->flags=0;
					data_ptr=my_tail->next;
					my_tail->next=NULL;
					while(data_ptr){
						my_tail->next=malloc(sizeof(struct data_st));
						my_tail=my_tail->next;
						memcpy(my_tail, t->z+(uintptr_t)data_ptr, sizeof(struct data_st));
						data_ptr=my_tail->next;
						my_tail->next=orig_next;
						my_tail->data=t->z+(uintptr_t)my_tail->data;
						my_tail->flags=0;
					}
					if(orig_next==NULL){
						this_phase->data_tail=my_tail;
					}
					data=this_phase->data->data;
					break;
			}
			break;
	}
	if(*data!=0x02){
		this_phase->state.status=DEADEND;
		return;
	}
	this_phase->state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=this_phase->data->len-1;
	this_phase->data_tail->next=malloc(sizeof(struct data_rt));
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	sprintf(p,"<img class=\"cns11643_img\" src=\"http://www.cns11643.gov.tw/AIDB/png.do?page=");
	TAILIZE(p);
	sprintf(p,"%X", (unsigned char)data[0]);
	TAILIZE(p);
	sprintf(p,"&code=");
	for(i=1;i<len;i++){
		TAILIZE(p);
		sprintf(p,"%02X", (unsigned char)data[i]);
	}
	TAILIZE(p);
	sprintf(p, "\" />");
	TAILIZE(p);
	len=p-buf;
	this_phase->data_tail->len=len;
	this_phase->data_tail->flags=F_FREE;
	this_phase->data_tail->data=malloc(len);
	memcpy(this_phase->data_tail->data, buf, len);

	return;
}
