#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p, buf[16]={0};
	unsigned int len, i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phasen-1];
	data=prev_phase->data->data;
	if(*data!=0x01){
		this_phase->state.status=DEADEND;
		return;
	}
	this_phase->state.status=NEXTPHASE;
	data+=1;
	len=prev_phase->data->len-1;

	this_phase->data_tail->next=malloc(sizeof(struct data_s));
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	p=buf;
	sprintf(p,"&#x%X",data[0]);
	for(i=1;i<len;i++){
		TAILIZE(p);
		sprintf(p,"%02X", data[i]);
	}
	TAILIZE(p);
	*p=';';
	len=strlen(buf);
	this_phase->data_tail->len=len;
	this_phase->data_tail->data=malloc(len);
	memcpy(this_phase->data_tail->data, buf, len);
}
