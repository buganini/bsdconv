/*
 * Reference:
 * http://icu-project.org/docs/papers/gb18030.html
 * http://source.icu-project.org/repos/icu/data/trunk/charset/data/xml/gb-18030-2000.xml
*/

#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
	uint32_t ucs;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->status=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r);
}

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->status=0;	\
	return;	\
}while(0);

struct gb18030_data {
	uint32_t beg;
	uint32_t end;
	uint32_t off;
};

static const struct gb18030_data gb18030_table[] = {
	{1688038, 1695139, 0x0452},
	{1696437, 1698546, 0x2643},
	{1700191, 1700955, 0x361B},
	{1701916, 1702800, 0x3CE1},
	{1703065, 1703535, 0x4160},
	{1703947, 1704319, 0x44D7},
	{1704636, 1705076, 0x478E},
	{1705179, 1705881, 0x49B8},
	{1706261, 1720686, 0x9FA6},
	{1720768, 1725062, 0xE865},
	{1725296, 1726325, 0xFA2A},
	{1726612, 1726637, 0xFFE6},
	{1876218, 2924793, 0x10000},
};

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	unsigned char d;
	unsigned char *c;
	struct data_st data;
	int max=sizeof(gb18030_table) / sizeof(struct gb18030_data) - 1;
	int min = 0;
	int mid;
	int i;
	union {
		unsigned char byte[4];
		uint32_t ucs4;
	} ucs;

	for(;this_phase->i<this_phase->curr->len;this_phase->i+=1){
		d=UCP(this_phase->curr->data)[this_phase->i];
		memcpy(&data, (char *)(this_phase->codec[this_phase->index].data_z+(uintptr_t)this_phase->state.data), sizeof(struct data_st));
		c=UCP(this_phase->codec[this_phase->index].data_z+(uintptr_t)data.data);
		next:
		switch(t->status){
			case 0:
				if(t->status<data.len){
					t->ucs=c[0]*10;
					t->status=1;
					goto next;
				}
				t->ucs=d*10;
				t->status=1;
				break;
			case 1:
				if(t->status<data.len){
					t->ucs+=c[1];
					t->ucs*=126;
					t->status=2;
					goto next;
				}
				t->ucs+=d;
				t->ucs*=126;
				t->status=2;
				break;
			case 2:
				if(t->status<data.len){
					t->ucs+=c[2];
					t->ucs*=10;
					t->status=3;
					goto next;
				}
				t->ucs+=d;
				t->ucs*=10;
				t->status=3;
				break;
			case 3:
				if(t->status<data.len){
					t->ucs+=c[3];
					t->status=0;
					goto next;
				}
				t->ucs+=d;
				t->status=0;
				if (t->ucs < gb18030_table[0].beg || t->ucs > gb18030_table[max].end){
					DEADEND();
				}else while (max >= min) {
					mid = (min + max) / 2;
					if (t->ucs > gb18030_table[mid].end)
						min = mid + 1;
					else if (t->ucs < gb18030_table[mid].beg)
						max = mid - 1;
					else{
						break;
					}
				}
				if(gb18030_table[mid].beg<=t->ucs && t->ucs<=gb18030_table[mid].end){
					ucs.ucs4=htobe32(gb18030_table[mid].off + (t->ucs - gb18030_table[mid].beg));
					for(i=0;ucs.byte[i]==0 && i<4;++i);
					DATA_MALLOC(this_phase->data_tail->next);
					this_phase->data_tail=this_phase->data_tail->next;
					this_phase->data_tail->next=NULL;
					this_phase->data_tail->len=5 - i;
					this_phase->data_tail->data=c=malloc(5 - i);
					this_phase->data_tail->flags=F_FREE;
					this_phase->state.status=NEXTPHASE;
					*c=0x01;
					c+=1;
					for(;i<4;++i,c+=1){
						*c=ucs.byte[i];
					}
					return;
				}else{
					DEADEND();
				}
				break;
			default:
				DEADEND();
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
