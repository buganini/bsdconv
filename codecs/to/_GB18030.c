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
#include "../../src/bsdconv.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

struct gb18030_data {
	uint32_t beg;
	uint32_t end;
	uint32_t off;
};

static const struct gb18030_data gb18030_table[] = {
	{0x0452,	0x200F,	1688038},
	{0x2643,	0x2E80,	1696437},
	{0x361B,	0x3917,	1700191},
	{0x3CE1,	0x4055,	1701916},
	{0x4160,	0x4336,	1703065},
	{0x44D7,	0x464B,	1703947},
	{0x478E,	0x4946,	1704636},
	{0x49B8,	0x4C76,	1705179},
	{0x9FA6,	0xD7FF,	1706261},
	{0xE865,	0xF92B,	1720768},
	{0xFA2A,	0xFE2F,	1725296},
	{0xFFE6,	0xFFFF,	1726612},
	{0x10000,	0x10FFFF,	1876218},
};

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p;
	unsigned int len;
	int max=sizeof(gb18030_table) / sizeof(struct gb18030_data) - 1;
	int min = 0;
	int mid;
	union {
		unsigned char byte[4];
		uint32_t num;
	} codepoint;
	int i;
	uint32_t ucs;
	uint32_t gb;
	data=ins->phase[ins->phase_index].curr->data;

	data+=1;
	len=ins->phase[ins->phase_index].curr->len-1;

	codepoint.num=0;
	for(i=0;(len-i)>0;++i){
		codepoint.byte[3-i]=data[len-i-1];
	}
	ucs=ntohl(codepoint.num);

	if (ucs < gb18030_table[0].beg || ucs > gb18030_table[max].end){
		ins->phase[ins->phase_index].state.status=DEADEND;
		return;
	}else while (max >= min) {
		mid = (min + max) / 2;
		if (ucs > gb18030_table[mid].end)
			min = mid + 1;
		else if (ucs < gb18030_table[mid].beg)
			max = mid - 1;
		else{
			break;
		}
	}
	if(gb18030_table[mid].beg<=ucs && ucs<=gb18030_table[mid].end){
		ins->phase[ins->phase_index].state.status=NEXTPHASE;
		DATA_MALLOC(ins->phase[ins->phase_index].data_tail->next);
		ins->phase[ins->phase_index].data_tail=ins->phase[ins->phase_index].data_tail->next;
		ins->phase[ins->phase_index].data_tail->next=NULL;
		ins->phase[ins->phase_index].data_tail->flags=F_FREE;

		gb=gb18030_table[mid].off + (ucs - gb18030_table[mid].beg);
		
		ins->phase[ins->phase_index].data_tail->len=4;
		p=ins->phase[ins->phase_index].data_tail->data=malloc(4);

		gb-=1687218;
		p[3]=0x30+gb%10;
		gb/=10;
		p[2]=0x81+gb%126;
		gb/=126;
		p[1]=0x30+gb%10;
		gb/=10;
		p[0]=0x81+gb;
		return;
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
		return;
	}
}
