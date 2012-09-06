#include "../../src/bsdconv.h"

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);

	if((this_phase->curr->flags & F_SKIP)){
		this_phase->curr->flags &= ~F_SKIP;
		this_phase->state.status=PASSTHRU;
	}else{
		this_phase->state.status=DEADEND;
	}

	return;
}
