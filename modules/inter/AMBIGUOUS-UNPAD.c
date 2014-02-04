#include "../../src/bsdconv.h"

#include "AMBIGUOUS.h"

struct my_s{
	char s;
	int dopad;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->s=0;
	r->dopad=1;
}

void cbctl(struct bsdconv_instance *ins, int ctl, void *ptr, size_t v){
	struct my_s *r=THIS_CODEC(ins)->priv;
	switch(ctl){
			break;
		case BSDCONV_CTL_AMBIGUOUS_PAD:
			r->dopad=v;
			break;
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	free(THIS_CODEC(ins)->priv);
}

void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;
	data=this_phase->curr->data;
	int pad;
	int max=sizeof(ambiguous) / sizeof(struct interval) - 1;
	int min = 0;
	int mid;
	uint32_t ucs=0;

	this_phase->state.status=NEXTPHASE;

	if(this_phase->curr->len>1 && data[0]==0x1){
		if(r->s==1 && data[1]==0xA0){
			r->s=0;
			return;
		}else{
			for(pad=1;pad<this_phase->curr->len;++pad){
				ucs<<=8;
				ucs|=data[pad];
			}

			pad=0;
			if (ucs < ambiguous[0].first || ucs > ambiguous[max].last){
				pad=0;
			}else while (max >= min) {
					mid = (min + max) / 2;
					if (ucs > ambiguous[mid].last)
						min = mid + 1;
					else if (ucs < ambiguous[mid].first)
						max = mid - 1;
					else{
						pad=1;
						break;
					}
			}
			if(pad && r->dopad){
				r->s=1;
			}
		}
		this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
	}else{
		r->s=0;
	}

	return;
}
