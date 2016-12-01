#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

// Ref: http://kanji-database.sourceforge.net/charcode/big5.html

struct my_s{
	int h;
	int x;
	int y;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r = malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv = r;
	r->h = 0;
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r = THIS_CODEC(ins)->priv;
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase = THIS_PHASE(ins);
	struct my_s *r = THIS_CODEC(ins)->priv;

	unsigned char d = UCP(this_phase->curr->data)[this_phase->i];

	if(r->h==0){
		if(d>=0xFA && d<=0xFE){
			r->h = d;
			r->x = 0xE000;
			r->y = 0xFA;
			this_phase->state.status = CONTINUE;
			return;
		}else if(d>=0x8E && d<=0xA0){
			r->h = d;
			r->x = 0xE311;
			r->y = 0x8E;
			this_phase->state.status = CONTINUE;
			return;
		}else if(d>=0x81 && d<=0x8D){
			r->h = d;
			r->x = 0xEEB8;
			r->y = 0x81;
			this_phase->state.status = CONTINUE;
			return;
		}else if(d>=0xC6 && d<=0xC8){
			r->h = d;
			r->x = 0xF672;
			r->y = 0xC6;
			this_phase->state.status = CONTINUE;
			return;
		}else{
			this_phase->state.status = DEADEND;
			return;
		}
	}else{
		uint32_t b = (r->h<<8)|d;
		if(
			(b>=0xFA40 && b<=0xFEFE)
			||
			(b>=0x8E40 && b<=0xA0FE)
			||
			(b>=0x8140 && b<=0x8DFE)
			||
			(b>=0xC6A1 && b<=0xC8FE)
		){
			uint32_t u = r->x + (157 * (r->h - r->y)) + (d<0x80?d-0x40:d-0x62);
			unsigned char *c;
			DATA_MALLOC(ins, this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->len=3;
			this_phase->data_tail->data=c=malloc(3);
			this_phase->data_tail->flags=F_FREE;
			this_phase->state.status=NEXTPHASE;
			c[0] = 0x01;
			c[1] = (u >> 8) & 0xFF;
			c[2] = u & 0xFF;
		}else{
			this_phase->state.status = DEADEND;
		}
		r->h = 0;
		return;
	}
}
