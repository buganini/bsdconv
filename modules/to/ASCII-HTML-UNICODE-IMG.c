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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void cbconv(struct bsdconv_instance *ins){
	char *data, *p, buf[128]={0};
	unsigned int len, i;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	data=this_phase->curr->data;
	if(*data!=0x01){
		this_phase->state.status=DEADEND;
		return;
	}
	this_phase->state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=this_phase->curr->len-1;
	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	sprintf(p,"<img class=\"unicode_img\" src=\"http://www.unicode.org/cgi-bin/refglyph?24-");
	TAILIZE(p);
	if(0<len){
		sprintf(p,"%X", (unsigned char)data[0]);
	}
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