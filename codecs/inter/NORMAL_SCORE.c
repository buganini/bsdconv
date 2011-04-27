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

#include "../../src/bsdconv.h"

struct interval {
	int first;
	int last;
	int score;
};

static const struct interval scoreboard[] = {
	{ 0x0, 0x7f, 5 },
	{ 0x3400, 0x4DB5, 3 },
	{ 0x4E00, 0x9FA5, 4 },
	{ 0x9FA6, 0x9FBB, 3 },
	{ 0xF900, 0xFA2D, 4 },
	{ 0xFA30, 0xFA6A, 4 },
	{ 0xFA70, 0xFAD9, 2 },
	{ 0x20000, 0x2A6D6, 1 },
	{ 0x2F800, 0x2FA1D, 1 },
};

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	data=this_phase->data->data;
	int i;
	int max=sizeof(scoreboard) / sizeof(struct interval) - 1;
	int min = 0;
	int mid;
	int ucs=0;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->data);
	this_phase->data->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	if(data[0]==0x1){
		for(i=1;i<this_phase->data->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}

		if (ucs < scoreboard[0].first || ucs > scoreboard[max].last){
			//noop
		}else while (max >= min) {
				mid = (min + max) / 2;
				if (ucs > scoreboard[mid].last)
					min = mid + 1;
				else if (ucs < scoreboard[mid].first)
					max = mid - 1;
				else{
					ins->score+=scoreboard[mid].score;
					break;
				}
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
