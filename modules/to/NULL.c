#include "../../src/bsdconv.h"

void cbconv(struct bsdconv_instance *ins){
	THIS_PHASE(ins)->state.status=NEXTPHASE;
	return;
}
