/*
 * Some code and table come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 * Copyright (c) 2009-2013 Kuan-Chung Chiu <buganini@gmail.com>
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

#include "../../src/bsdconv.h"

static void decomposeHangul(uint32_t ucs, struct bsdconv_instance *ins);

int
	SBase = 0xAC00,
	LBase = 0x1100, VBase = 0x1161, TBase = 0x11A7,
	LCount = 19, VCount = 21, TCount = 28,
	NCount = 588, //VCount * TCount
	SCount = 11172; //LCount * NCount


void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	data=this_phase->curr->data;
	int i;
	uint32_t ucs=0;

	if(data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		int SIndex  = ucs - SBase;
		if(SIndex >= 0 && SIndex < SCount){
			decomposeHangul(ucs, ins);
		}else{
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			*(this_phase->data_tail)=*(this_phase->curr);
			this_phase->curr->flags &= ~F_FREE;
			this_phase->data_tail->next=NULL;
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}

static void decomposeHangul(uint32_t ucs, struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	int SIndex  = ucs - SBase;
	if(SIndex >= 0 && SIndex < SCount){
		int L = LBase + SIndex / NCount;
		int V = VBase + (SIndex % NCount) / TCount;
		int T = TBase + SIndex % TCount;

		decomposeHangul(L, ins);
		decomposeHangul(V, ins);
		if(T != TBase)
			decomposeHangul(T, ins);
	}else{
		int i;
		unsigned char *p;
		unsigned char stack[8];
		int stack_len=0;
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		while(ucs && stack_len<sizeof(stack)){
			stack[stack_len] = ucs & 0xff;
			ucs >>= 8;
			stack_len += 1;
		}
		this_phase->data_tail->len=stack_len+=1;
		this_phase->data_tail->data=malloc(this_phase->data_tail->len);
		p=this_phase->data_tail->data;
		*p=1;
		p+=1;
		stack_len-=1;
		for(i=0;i<stack_len;i+=1){
			*p=stack[stack_len-i-1];
			p+=1;
		}
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->next=NULL;
	}
}
