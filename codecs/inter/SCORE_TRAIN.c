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
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif
#include "../../src/bsdconv.h"

struct my_s{
	FILE *bak;
	FILE *score;
	FILE *list;
};

void cbcreate(struct bsdconv_instance *ins){
	struct my_s *r=malloc(sizeof(struct my_s));
	char buf[256]={0};
	char *p=getenv("BSDCONV_SCORE");
	if(p==NULL){
		strcpy(buf,getenv("HOME"));
		strcat(buf,"/.bsdconv.score");
		p=buf;
	}
	r->bak=fopen(p,"a"); //ensure file existence
	fclose(r->bak);
	r->bak=r->score=fopen(p,"rb+");
	r->list=NULL;
	CURRENT_CODEC(ins)->priv=r;
	
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	fclose(r->bak);
	free(r);
}

void cbctl(struct bsdconv_instance *ins, int ctl, void *ptr, size_t v){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	switch(ctl){
		case BSDCONV_ATTACH_SCORE:
			r->score=ptr;
			break;
		case BSDCONV_ATTACH_OUTPUT_FILE:
			r->list=ptr;
			break;
	}
}

void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	data=this_phase->curr->data;
	unsigned char v=0;
	int i;
	int ucs=0;
	uint32_t ucs4;

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
		fseek(r->score, ucs*sizeof(unsigned char), SEEK_SET);
		fread(&v, sizeof(unsigned char), 1, r->score);
		if(v==0 && r->list){
			ucs4=htonl(ucs);
			fwrite(&ucs4, sizeof(uint32_t), 1, r->list);
		}
		if(v<3){
			v+=1;
			fseek(r->score, ucs*sizeof(unsigned char), SEEK_SET);
			fwrite(&v, sizeof(unsigned char), 1, r->score);
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
