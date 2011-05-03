#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];

	if(this_phase->curr->len>1 && CP(this_phase->curr->data)[0]==0x1b){
		this_phase->curr->flags |= F_SKIP;
		ins->phase[ins->phase_index].state.status=PASSTHRU;
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
	}

	return;
}
