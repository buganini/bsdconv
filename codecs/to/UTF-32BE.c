#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	unsigned int len, i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phasen-1];
	data=prev_phase->data->data;

	this_phase->state.status=NEXTPHASE;
	data+=1;
	len=prev_phase->data->len-1;

	this_phase->data_tail->next=malloc(sizeof(struct data_s));
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->len=4;
	this_phase->data_tail->data=malloc(4);
	for(i=0;i<4-len;++i){
		this_phase->data_tail->data[i]=0x0;
	}
	memcpy(&this_phase->data_tail->data[i], data, len);
}
