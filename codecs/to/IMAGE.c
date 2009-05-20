#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p, buf[128]={0};
	unsigned int len, i;
	data=ins->to_data->data;
	switch(*data){
		case 0x01:
		case 0x02:
			break;
		default:
			ins->to_state.status=DEADEND;
			return;
	}
	ins->to_state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=ins->to_data->len-1;
	ins->out_data_tail->next=malloc(sizeof(struct data_s));
	ins->out_data_tail=ins->out_data_tail->next;
	ins->out_data_tail->next=NULL;
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
			ins->out_data_tail->len=len;
			ins->out_data_tail->data=malloc(len);
			memcpy(ins->out_data_tail->data, buf, len);
			break;
	}
	return;
}
