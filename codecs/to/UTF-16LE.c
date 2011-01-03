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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define bb11011000 0xD8
#define bb00000011 0x03
#define bb11000000 0xC0
#define bb00111111 0x3F
#define bb11011100 0xDC

#define SWAP(a,b,i) ((i)=(a), (a)=(b), (b)=(i))

void callback(struct bsdconv_instance *ins){
	char *data, *p, c;
	unsigned int len, i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	data=this_phase->data->data;

	data+=1;
	if(this_phase->data->len > 3){
		this_phase->state.status=NEXTPHASE;

		this_phase->data_tail->next=malloc(sizeof(struct data_rt));
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=4;
		this_phase->data_tail->flags=F_FREE;
		p=this_phase->data_tail->data=malloc(4);

		c=*data-1;
		*p=bb11011000;
		*p |= (c >> 2) & bb00000011;
		++p;
		*p=(c << 6) & bb11000000;
		++data;
		*p |= (*data >> 2) & bb00111111;
		++p;
		*p=bb11011100;
		*p |= *data & bb00000011;
		++p;
		++data;
		*p=*data;

		data=this_phase->data_tail->data;

		SWAP(data[0],data[1],i);
		SWAP(data[2],data[3],i);
	}else{
		this_phase->state.status=NEXTPHASE;
		len=this_phase->data->len-1;

		this_phase->data_tail->next=malloc(sizeof(struct data_rt));
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=2;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(2);
		for(i=0;i<2-len;++i){
			CP(this_phase->data_tail->data)[i]=0x0;
		}
		memcpy(CP(this_phase->data_tail->data)+i, data, len);
		data=this_phase->data_tail->data;

		SWAP(data[0],data[1],i);
	}
	return;
}
