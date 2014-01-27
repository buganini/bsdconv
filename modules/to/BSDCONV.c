#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void cbconv(struct bsdconv_instance *ins){
	int i;
	char *p;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);

	this_phase->state.status=NEXTPHASE;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->flags=F_FREE;

	this_phase->data_tail->len=this_phase->curr->len*2;
	p=this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
	for(i=0;i<this_phase->curr->len;++i){
		sprintf(p,"%02X", UCP(this_phase->curr->data)[i]);
		TAILIZE(p);
	}
}
