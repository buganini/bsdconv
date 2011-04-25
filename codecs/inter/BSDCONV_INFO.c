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

#include <stdio.h>
#include "../../src/bsdconv.h"

void *cbcreate(void){
	return malloc(sizeof(char));
}

void cbinit(struct bsdconv_codec_t *cdc, char *r){
	*r=0;
}

void cbdestroy(void *r){
	free(r);
}

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	char *r=this_phase->codec[this_phase->index].priv;
	data=this_phase->data->data;
	char seq[]={
		0x1b,0x5b,0x41, // UP
		0x1b,0x5b,0x41, // UP
		0x1b,0x5b,0x42, // DOWN
		0x1b,0x5b,0x42, // DOWN
		0x1b,0x5b,0x44, // LEFT
		0x1b,0x5b,0x44, // LEFT
		0x1b,0x5b,0x43, // RIGHT
		0x1b,0x5b,0x43, // RIGHT
		0x42, // B
		0x41 // A
		};

	if(this_phase->data->len==2 && data[0]==0x1){
		if(data[1]==seq[(int)(*r)]){
			++(*r);
		}else{
			*r=0;
		}
	}else{
		*r=0;
	}

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->data);
	this_phase->data->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	if(*r==sizeof(seq)){
		fprintf(stderr, "\r\nDecoding failure: %u\r\n", ins->ierr);
		fprintf(stderr, "Encoding failure: %u\r\n", ins->oerr);
		*r=0;
	}
	this_phase->state.status=NEXTPHASE;
}
