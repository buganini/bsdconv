#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	ins->phase[ins->phase_index].state.status=PASSTHRU;
	return;
}
