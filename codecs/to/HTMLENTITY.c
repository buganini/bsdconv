#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instruction *ins){
	unsigned char *data, *p, buf[16]={0};
	unsigned int len, i;
	data=ins->to_data->data;
	if(*data!=0x01){
		ins->to_state.status=DEADEND;
		return;
	}
	ins->to_state.status=NEXTPHASE;
	data+=1;
	len=ins->to_data->len-1;

	ins->out_data_tail->next=malloc(sizeof(struct data_s));
	ins->out_data_tail=ins->out_data_tail->next;
	ins->out_data_tail->next=NULL;

	p=buf;
	sprintf(p,"&#x");
	TAILIZE(p);
	sprintf(p,"%X", data[0]);
	TAILIZE(p);
	for(i=1;i<len;i++){
		sprintf(p,"%02X", data[i]);
		TAILIZE(p);
	}
	*p=';';
	len=strlen(buf);
	ins->out_data_tail->len=len;
	ins->out_data_tail->data=malloc(len);
	memcpy(ins->out_data_tail->data, buf, len);
}
