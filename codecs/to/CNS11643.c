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
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

void *cbcreate(void){
	return bsdconv_create("CNS11643");
}

void cbdestroy(void *p){
	bsdconv_destroy(p);
}

void callback(struct bsdconv_instance *ins){
	char *data;
	unsigned int len;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct bsdconv_instance *cns=this_phase->codec[this_phase->index].priv;
	struct data_rt *data_p=this_phase->data;
	data=this_phase->data->data;

	/* exclude ASCII*/
	if(ins->phase[ins->phase_index].data->len==2 && (data[1] & bb10000000)==0){
		this_phase->state.status=DEADEND;
		return;
	}

	switch(*data){
		case 0x01:
			if(cns!=NULL){
				bsdconv_init(cns);
				cns->input.data=data;
				cns->input.len=this_phase->data->len;
				cns->input.flags=F_SKIP;
				cns->input.next=NULL;
				cns->flush=1;
				bsdconv(cns);
				data_p=cns->phase[cns->phasen].data_head->next;
				cns->phase[cns->phasen].data_head->next=NULL;
				data=data_p->data;
			}
			if(*data==0x02){
				goto converted;
			}else{
				this_phase->state.status=DEADEND;
				if(data_p!=this_phase->data)
					DATA_FREE(data_p);
				return;
			}
		case 0x02:
			converted:
			len=ins->phase[ins->phase_index].data->len-1;

			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;

			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=4;
			this_phase->data_tail->data=malloc(4);
			memcpy(this_phase->data_tail->data, data, this_phase->data_tail->len);
			CP(this_phase->data_tail->data)[0]=0;
			this_phase->state.status=NEXTPHASE;

			if(data_p!=this_phase->data)
				DATA_FREE(data_p);

			return;
		default:
			this_phase->state.status=DEADEND;
			return;
	}
}
