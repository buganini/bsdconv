#include "../../src/bsdconv.h"

#define HALF 1
#define FULL 2
#define AMBI -1

#include "_WIDTH.h"

struct my_s{
	bsdconv_counter_t *full;
	bsdconv_counter_t *half;
	bsdconv_counter_t *ambi;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));

	r->full=bsdconv_counter(ins, "FULL");
	r->half=bsdconv_counter(ins, "HALF");
	r->ambi=bsdconv_counter(ins, "AMBI");
	return 0;
}

void cbconv(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	data=this_phase->curr->data;
	int i;
	int max=sizeof(width_table) / sizeof(struct width_interval) - 1;
	int min = 0;
	int mid;
	uint32_t ucs=0;

	this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	if(data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		if (ucs < width_table[0].beg || ucs > width_table[max].end){
			//noop
		}else while (max >= min) {
			mid = (min + max) / 2;
			if (ucs > width_table[mid].end)
				min = mid + 1;
			else if (ucs < width_table[mid].beg)
				max = mid - 1;
			else{
				switch(width_table[mid].width){
					case FULL:
						*(r->full)+=1;
						break;
					case HALF:
						*(r->half)+=1;
						break;
					case AMBI:
						*(r->ambi)+=1;
						break;
				}
				break;
			}
		}
	}

	this_phase->state.status=NEXTPHASE;
	return;
}


void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r);
}
