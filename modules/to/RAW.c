#include <stdlib.h>
#include "../../src/bsdconv.h"

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	int i;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->len=ins->phase[ins->phase_index].curr->len-1;
	this_phase->data_tail->flags=F_FREE;
	this_phase->data_tail->data=malloc(this_phase->data_tail->len);
	for(i=0;i<this_phase->data_tail->len;++i){
		CP(this_phase->data_tail->data)[i]=CP(this_phase->curr->data)[i+1];
	}
	this_phase->state.status=NEXTPHASE;
	return;
}
