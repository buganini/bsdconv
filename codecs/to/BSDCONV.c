#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	int i;
	char *p;
	ins->to_state.status=NEXTPHASE;

	ins->out_data_tail->next=malloc(sizeof(struct data_s));
	ins->out_data_tail=ins->out_data_tail->next;
	ins->out_data_tail->next=NULL;

	ins->out_data_tail->len=ins->to_data->len*2;;
	p=ins->out_data_tail->data=malloc(ins->out_data_tail->len);
	for(i=0;i<ins->out_data_tail->len;++i){
		sprintf(p,"%02X",ins->to_data->data[i]);
		TAILIZE(p);
	}
}
