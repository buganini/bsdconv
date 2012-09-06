#include "../../src/bsdconv.h"

void cbconv(struct bsdconv_instance *ins){
	CURRENT_PHASE(ins)->state.status=PASSTHRU;
	return;
}
