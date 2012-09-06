#include "../../src/bsdconv.h"

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);

	if(this_phase->curr->len>1 && CP(this_phase->curr->data)[0]==0x1b){
		this_phase->curr->flags |= F_SKIP;
		ins->phase[ins->phase_index].state.status=PASSTHRU;
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
	}

	return;
}
