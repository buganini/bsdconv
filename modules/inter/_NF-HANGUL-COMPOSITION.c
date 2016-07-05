#include "../../src/bsdconv.h"

#define SBase	0xAC00
#define LBase	0x1100
#define VBase	0x1161
#define TBase	0x11A7
#define LCount	19
#define VCount	21
#define TCount	28
#define NCount	(VCount * TCount)
#define SCount	(LCount * NCount)

struct my_s {
	int status;
	uint32_t pending;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->status=0;
}

void cbconv(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	data=this_phase->curr->data;
	int i;
	uint32_t ucs=0;
	int LIndex, VIndex, TIndex;

	if(data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}

		switch(r->status){
			case 0:
				LIndex = ucs - LBase;
				if (0 <= LIndex && LIndex < LCount){
					r->pending = ucs;
					r->status=1;
					this_phase->state.status=SUBMATCH;
				}else{
					this_phase->state.status=DEADEND;
				}
				return;
			case 1:
				LIndex = r->pending - LBase;
				VIndex = ucs - VBase;
				if (0 <= VIndex && VIndex < VCount) {
					r->pending = (SBase + (LIndex * VCount + VIndex) * TCount);
					int SIndex = r->pending - SBase;
					if(0 <= SIndex && SIndex < SCount && (SIndex % TCount) == 0){
						r->status=2;
						this_phase->state.status=SUBMATCH;
					}else{
						cbflush(ins);

						LIndex = ucs - LBase;
						if (0 <= LIndex && LIndex < LCount){
							r->pending = ucs;
							r->status=1;
							this_phase->state.status=SUBMATCH;
						}else{
							this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
							this_phase->data_tail=this_phase->data_tail->next;
							this_phase->data_tail->next=NULL;
						}
					}
				}else{
					cbflush(ins);

					LIndex = ucs - LBase;
					if (0 <= LIndex && LIndex < LCount){
						r->pending = ucs;
						r->status=1;
						this_phase->state.status=SUBMATCH;
					}else{
						LIndex = ucs - LBase;
						if (0 <= LIndex && LIndex < LCount){
							r->pending = ucs;
							r->status=1;
							this_phase->state.status=SUBMATCH;
						}else{
							this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
							this_phase->data_tail=this_phase->data_tail->next;
							this_phase->data_tail->next=NULL;
						}
					}
				}
				return;
			case 2:
				TIndex = ucs - TBase;
				if (0 < TIndex && TIndex < TCount) {
					r->pending += TIndex;
					cbflush(ins);
				}else{
					cbflush(ins);

					//restart

					//unrolled
					//this_phase->state.status=YIELD;
					//return;

					LIndex = ucs - LBase;
					if (0 <= LIndex && LIndex < LCount){
						r->pending = ucs;
						r->status=1;
						this_phase->state.status=SUBMATCH;
					}else{
						this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
						this_phase->data_tail=this_phase->data_tail->next;
						this_phase->data_tail->next=NULL;
					}
				}
				return;
		}
	}else{
		cbflush(ins);
		this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
	}
	return;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	this_phase->state.status=NEXTPHASE;

	if(r->status==0)
		return;

	uint32_t ucs=r->pending;
	int i;
	unsigned char *p;
	unsigned char stack[8];
	int stack_len=0;
	DATA_MALLOC(ins, this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	while(ucs && stack_len<sizeof(stack)){
		stack[stack_len] = ucs & 0xff;
		ucs >>= 8;
		stack_len += 1;
	}
	this_phase->data_tail->len=stack_len+=1;
	this_phase->data_tail->data=malloc(this_phase->data_tail->len);
	p=this_phase->data_tail->data;
	*p=1;
	p+=1;
	stack_len-=1;
	for(i=0;i<stack_len;i+=1){
		*p=stack[stack_len-i-1];
		p+=1;
	}
	this_phase->data_tail->flags=F_FREE;
	this_phase->data_tail->next=NULL;

	r->status=0;
}
