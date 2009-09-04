#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	int i;
	char *p;
	ins->phase[ins->phasen].state.status=NEXTPHASE;

	ins->phase[ins->phasen].data_tail->next=malloc(sizeof(struct data_s));
	ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_tail->next;
	ins->phase[ins->phasen].data_tail->next=NULL;

	ins->phase[ins->phasen].data_tail->len=ins->phase[ins->phasen-1].data->len*2;;
	p=ins->phase[ins->phasen].data_tail->data=malloc(ins->phase[ins->phasen].data_tail->len);
	for(i=0;i<ins->phase[ins->phasen-1].data_tail->len;++i){
		sprintf(p,"%02X",ins->phase[ins->phasen-1].data->data[i]);
		TAILIZE(p);
	}
}
