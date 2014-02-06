#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void cbconv(struct bsdconv_instance *ins){
	char *data, *p, buf[128]={0};
	unsigned int len, i;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	data=this_phase->curr->data;
	if(*data!=0x01){
		this_phase->state.status=DEADEND;
		return;
	}
	this_phase->state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=this_phase->curr->len-1;
	DATA_MALLOC(ins, this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	sprintf(p,"<img class=\"unicode_img\" src=\"http://www.unicode.org/cgi-bin/refglyph?24-");
	TAILIZE(p);
	if(0<len){
		sprintf(p,"%X", (unsigned char)data[0]);
	}
	for(i=1;i<len;i++){
		TAILIZE(p);
		sprintf(p,"%02X", (unsigned char)data[i]);
	}
	TAILIZE(p);
	sprintf(p, "\" />");
	TAILIZE(p);
	len=p-buf;
	this_phase->data_tail->len=len;
	this_phase->data_tail->flags=F_FREE;
	this_phase->data_tail->data=malloc(len);
	memcpy(this_phase->data_tail->data, buf, len);

	return;
}
