#include <stdlib.h>
#include "../../src/bsdconv.h"

void callback(struct bsdconv_instruction *ins){
	ins->out_data_tail->next=malloc(sizeof(struct data_s));
	ins->out_data_tail=ins->out_data_tail->next;
	ins->out_data_tail->next=NULL;
	ins->out_data_tail->len=1;
	ins->out_data_tail->data=malloc(1);
	*ins->out_data_tail->data='?';

	ins->to_state.status=NEXTPHASE;
	return;
}
