#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p, buf[16]={0};
	unsigned int len, i;
	data=ins->phase[ins->phasen-1].data->data;
	if(*data!=0x01){
		ins->phase[ins->phasen].state.status=DEADEND;
		return;
	}
	ins->phase[ins->phasen].state.status=NEXTPHASE;
	data+=1;
	len=ins->phase[ins->phasen-1].data->len-1;

	ins->phase[ins->phasen].data_tail->next=malloc(sizeof(struct data_s));
	ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_tail->next;
	ins->phase[ins->phasen].data_tail->next=NULL;

	p=buf;
	sprintf(p,"\\u%x",data[0]);
	for(i=1;i<len;i++){
		TAILIZE(p);
		sprintf(p,"%02x", data[i]);
	}
	len=strlen(buf);
	ins->phase[ins->phasen].data_tail->len=len;
	ins->phase[ins->phasen].data_tail->data=malloc(len);
	memcpy(ins->phase[ins->phasen].data_tail->data, buf, len);
}
