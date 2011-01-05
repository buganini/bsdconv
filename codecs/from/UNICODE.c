#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phase_index-1];
	struct data_rt *data_ptr;

	if((this_phase->data->flags & F_SKIP) && this_phase->data->len>1 && CP(this_phase->data->data)[0]==0x01){
		this_phase->bak=this_phase->data->next;
		while(prev_phase->data_head->next!=this_phase->data){
			data_ptr=prev_phase->data_head->next->next;
			DATA_FREE(prev_phase->data_head->next);
			prev_phase->data_head->next=data_ptr;
		}
		this_phase->data_tail->next=prev_phase->data_head->next;
		if(prev_phase->data_tail==prev_phase->data_head->next){
			prev_phase->data_tail=prev_phase->data_head;
		}
		prev_phase->data_head->next=prev_phase->data_head->next->next;
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->flags &= ~F_SKIP;
		this_phase->data_tail->next=NULL;

		this_phase->data_head->len=0;
		this_phase->data=prev_phase->data_head;
		ins->phase[ins->phase_index].state.status=PASSTHRU;
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
	}

	return;
}
