#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];

	if((this_phase->data->flags & F_SKIP)){
		this_phase->data->flags &= ~F_SKIP;
		ins->phase[ins->phase_index].state.status=PASSTHRU;
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
	}

	return;
}
