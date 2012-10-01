/*
 * Reference: http://blog.oasisfeng.com/2006/10/19/full-cjk-unicode-range/
 * Some code come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 *
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	FILE *bak;
	FILE *score;
};

void cbcreate(struct bsdconv_instance *ins){
	struct my_s *r=malloc(sizeof(struct my_s));
	char buf[256]={0};
	char *p=getenv("BSDCONV_SCORE");
	if(p==NULL){
		strcpy(buf,getenv("HOME"));
		strcat(buf,"/.bsdconv.score");
		p=buf;
		r->bak=r->score=fopen(p,"r+");
	}else{
		r->bak=NULL;
		r->score=fopen(p,"r");
	}
	CURRENT_CODEC(ins)->priv=r;
}

void cbctl(struct bsdconv_instance *ins, int ctl, void *ptr, size_t v){
	switch(ctl){
		case BSDCONV_ATTACH_SCORE:
			CURRENT_CODEC(ins)->priv=ptr;
			break;
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	if(r->bak)
		fclose(r->bak);
	free(r);
}

struct interval {
	int first;
	int last;
	double score;
};

static const struct interval scoreboard[] = {
	{ 0x0, 0x7F, 4 },	//ASCII
	{ 0x3000, 0x303F, 4 },	//CJK punctuation
	{ 0x3040, 0x309F, 5 },	//Japanese hiragana
	{ 0x30A0, 0x30FF, 5 },	//Japanese katakana
	{ 0x3100, 0x312F, 4 },	//Chinese Bopomofo
	{ 0x3400, 0x4DB5, 3 },	//CJK Unified Ideographs Extension A	;Unicode3.0
	{ 0x4E00, 0x6FFF, 5 },	//CJK Unified Ideographs	;Unicode 1.1	;HF
	{ 0x7000, 0x9FA5, 4 },	//CJK Unified Ideographs	;Unicode 1.1	;LF
	{ 0x9FA6, 0x9FBB, 3 },	//CJK Unified Ideographs	;Unicode 4.1
	{ 0xAC00, 0xD7AF, 3 },	//Korean word
	{ 0xF900, 0xFA2D, 4 },	//CJK Compatibility Ideographs	;Unicode 1.1
	{ 0xFA30, 0xFA6A, 4 },	//CJK Compatibility Ideographs	;Unicode 3.2
	{ 0xFA70, 0xFAD9, 2 },	//CJK Compatibility Ideographs	;Unicode 4.1
	{ 0xFF00, 0xFFEF, 3},	//Fullwidth ASCII, punctuation, Japanese, Korean
	{ 0x20000, 0x2A6D6, 1 },//CJK Unified Ideographs Extension B	;Unicode 3.1
	{ 0x2F800, 0x2FA1D, 1 },//CJK Compatibility Supplement	;Unicode 3.1
};

void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	FILE *fp=CURRENT_CODEC(ins)->priv;
	data=this_phase->curr->data;
	int i;
	int max=sizeof(scoreboard) / sizeof(struct interval) - 1;
	int min = 0;
	int mid;
	int ucs=0;
	unsigned char v=0;

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

		if(fp==NULL){
			if (ucs < scoreboard[0].first || ucs > scoreboard[max].last){
				//noop
			}else while (max >= min) {
					mid = (min + max) / 2;
					if (ucs > scoreboard[mid].last)
						min = mid + 1;
					else if (ucs < scoreboard[mid].first)
						max = mid - 1;
					else{
						if(ins->score+scoreboard[mid].score < INT_MAX)
							ins->score+=scoreboard[mid].score;
						break;
					}
			}
		}else{
			fseek(fp, ucs*sizeof(unsigned char), SEEK_SET);
			fread(&v, sizeof(unsigned char), 1, fp);
			ins->score+=v;
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
