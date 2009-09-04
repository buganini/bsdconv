#include <stdlib.h>
#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	ins->phase[ins->phasen].data_tail->next=malloc(sizeof(struct data_s));
	ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_tail->next;
	ins->phase[ins->phasen].data_tail->next=NULL;
	ins->phase[ins->phasen].data_tail->len=1;
	ins->phase[ins->phasen].data_tail->data=malloc(1);
	*ins->phase[ins->phasen].data_tail->data='?';

	ins->phase[ins->phasen].state.status=NEXTPHASE;
	return;
}
