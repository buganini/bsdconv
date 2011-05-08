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

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

void * cbcreate(void){
	char buf[256]={0};
	FILE *fp;
	char *p=getenv("BSDCONV_FREQ");
	if(p==NULL){
		strcpy(buf,getenv("HOME"));
		strcat(buf,"/.bsdconv.freq");
		p=buf;
	}
	fp=fopen(p,"r+");
	if(fp==NULL){
		fp=fopen(p,"w+");
	}
	return fp;
}

void cbdestroy(FILE *fp){
	fclose(fp);
}

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	FILE *fp=this_phase->codec[this_phase->index].priv;
	data=this_phase->curr->data;
	uint32_t v=0;
	int i;
	int ucs=0;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	if(data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		fseek(fp, ucs*sizeof(uint32_t), SEEK_SET);
		fread(&v, sizeof(uint32_t), 1, fp);
		if(v<UINT32_MAX)
			v+=1;
		fwrite(&v, sizeof(uint32_t), 1, fp);
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
