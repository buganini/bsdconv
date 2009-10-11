/*
 * Copyright (c) 2009 Kuan-Chung Chiu <buganini@gmail.com>
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
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	int i;
	char *p;
	ins->phase[ins->phasen].state.status=NEXTPHASE;

	ins->phase[ins->phasen].data_tail->next=malloc(sizeof(struct data_s));
	ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_tail->next;
	ins->phase[ins->phasen].data_tail->next=NULL;

	ins->phase[ins->phasen].data_tail->len=ins->phase[ins->phasen-1].data->len*2;;
	p=ins->phase[ins->phasen].data_tail->data=malloc(ins->phase[ins->phasen].data_tail->len);
	for(i=0;i<ins->phase[ins->phasen-1].data_tail->len;++i){
		sprintf(p,"%02X",ins->phase[ins->phasen-1].data->data[i]);
		TAILIZE(p);
	}
}
