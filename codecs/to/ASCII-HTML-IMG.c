#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p, buf[128]={0};
	unsigned int len, i;
	data=ins->phase[ins->phasen-1].data->data;
	switch(*data){
		case 0x01:
		case 0x02:
			break;
		default:
			ins->phase[ins->phasen].state.status=DEADEND;
			return;
	}
	ins->phase[ins->phasen].state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=ins->phase[ins->phasen-1].data->len-1;
	ins->phase[ins->phasen].data_tail->next=malloc(sizeof(struct data_s));
	ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_tail->next;
	ins->phase[ins->phasen].data_tail->next=NULL;
	switch(i){
		case 0x01:
			sprintf(p,"<img class=\"unicode_img\" src=\"http://www.unicode.org/cgi-bin/refglyph?24-");
			for(i=0;i<len;i++){
				TAILIZE(p);
				sprintf(p,"%02X", data[i]);
			}
			TAILIZE(p);
			sprintf(p, "\" />");
			TAILIZE(p);
			len=p-buf;
			ins->phase[ins->phasen].data_tail->len=len;
			ins->phase[ins->phasen].data_tail->data=malloc(len);
			memcpy(ins->phase[ins->phasen].data_tail->data, buf, len);
			break;
	}
	return;
}
