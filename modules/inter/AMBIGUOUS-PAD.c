#include "../../src/bsdconv.h"

#include "AMBIGUOUS.h"

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=malloc(sizeof(int));

	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	int *r=THIS_CODEC(ins)->priv;
	*r=1;
}

void cbctl(struct bsdconv_instance *ins, int ctl, void *ptr, size_t v){
	int *r=THIS_CODEC(ins)->priv;
	switch(ctl){
			break;
		case BSDCONV_CTL_AMBIGUOUS_PAD:
			*r=v;
			break;
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	int *r=THIS_CODEC(ins)->priv;
	free(r);
}
void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	int *dopad=THIS_CODEC(ins)->priv;
	data=this_phase->curr->data;
	int pad;

	int max=sizeof(ambiguous) / sizeof(struct interval) - 1;
	int min = 0;
	int mid;
	char *space="\x01\xA0";
	uint32_t ucs=0;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	if(this_phase->curr->len>0 && data[0]==0x1){
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
		if(pad && *dopad){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->len=2;
			this_phase->data_tail->data=space;
			this_phase->data_tail->flags=0;
			this_phase->data_tail->next=NULL;
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}
