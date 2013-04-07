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

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

int cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	CURRENT_CODEC(ins)->priv=bsdconv_create("CNS11643");
	return 1;
}

void cbdestroy(struct bsdconv_instance *ins){
	void *p=CURRENT_CODEC(ins)->priv;
	if(p!=NULL)
		bsdconv_destroy(p);
}

void cbconv(struct bsdconv_instance *ins){
	char *data, *p, buf[128]={0};
	unsigned int len, i;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct bsdconv_instance *cns=this_phase->codec[this_phase->index].priv;
	struct data_rt *data_p=this_phase->curr;
	data=this_phase->curr->data;
	switch(*data){
		case 0x01:
			if(cns!=NULL){
				bsdconv_init(cns);
				cns->input.data=data;
				cns->input.len=this_phase->curr->len;
				cns->input.flags=0;
				cns->input.next=NULL;
				cns->flush=1;
				bsdconv(cns);
				data_p=cns->phase[cns->phasen].data_head->next;
				cns->phase[cns->phasen].data_head->next=NULL;
			}
			break;
	}
	data=data_p->data;
	if(*data!=0x02){
		this_phase->state.status=DEADEND;
		if(data_p!=this_phase->curr)
			DATA_FREE(data_p);
		return;
	}
	this_phase->state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=data_p->len-1;
	DATA_MALLOC(this_phase->data_tail->next);
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

	if(data_p!=this_phase->curr)
		DATA_FREE(data_p);
	return;
}
