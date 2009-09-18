#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void callback(struct bsdconv_instance *ins){
	int i;
	ins->phase[ins->phasen].state.status=NEXTPHASE;

	for(i=0;i<ins->phase[ins->phasen-1].data_tail->len;++i){
		printf("%02X",ins->phase[ins->phasen-1].data->data[i]);
	}
	printf("\n");
}
