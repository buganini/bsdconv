#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	CURRENT_PHASE(ins)->state.status=PASSTHRU;
	return;
}
