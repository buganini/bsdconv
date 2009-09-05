#include <stdlib.h>
#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	this_phase->data_tail->next=malloc(sizeof(struct data_s));
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->len=1;
	this_phase->data_tail->data=malloc(1);
	*this_phase->data_tail->data='?';

	this_phase->state.status=NEXTPHASE;
	return;
}
