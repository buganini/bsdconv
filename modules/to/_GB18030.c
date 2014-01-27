#include <stdlib.h>
#include "../../src/bsdconv.h"

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

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
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
	data=this_phase->curr->data;

	data+=1;
	len=this_phase->curr->len-1;

	codepoint.num=0;
	for(i=0;(len-i)>0;++i){
		codepoint.byte[3-i]=data[len-i-1];
	}
	ucs=be32toh(codepoint.num);

	if (ucs < gb18030_table[0].beg || ucs > gb18030_table[max].end){
		this_phase->state.status=DEADEND;
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
		this_phase->state.status=NEXTPHASE;
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->flags=F_FREE;

		gb=gb18030_table[mid].off + (ucs - gb18030_table[mid].beg);

		this_phase->data_tail->len=4;
		p=this_phase->data_tail->data=malloc(4);

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
		this_phase->state.status=DEADEND;
		return;
	}
}
