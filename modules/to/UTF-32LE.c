#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define SWAP(a,b,i) ((i)=(a), (a)=(b), (b)=(i))

void cbconv(struct bsdconv_instance *ins){
	char *data;
	unsigned int len, i;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	data=this_phase->curr->data;

	this_phase->state.status=NEXTPHASE;
	data+=1;
	len=this_phase->curr->len-1;

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->len=4;
	this_phase->data_tail->flags=F_FREE;
	this_phase->data_tail->data=malloc(4);
	for(i=0;i<4-len;++i){
		CP(this_phase->data_tail->data)[i]=0x0;
	}
	memcpy(CP(this_phase->data_tail->data)+i, data, len);
	data=this_phase->data_tail->data;
	SWAP(data[0],data[3],i);
	SWAP(data[1],data[2],i);
	return;
}
