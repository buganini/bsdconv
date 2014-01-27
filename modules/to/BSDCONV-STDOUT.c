#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void cbconv(struct bsdconv_instance *ins){
	int i;
	ins->phase[ins->phase_index].state.status=NEXTPHASE;

	for(i=0;i<ins->phase[ins->phase_index].curr->len;++i){
		printf("%02X",UCP(ins->phase[ins->phase_index].curr->data)[i]);
	}
	if(ins->phase[ins->phase_index].curr->flags){
		printf(" (");
		if(ins->phase[ins->phase_index].curr->flags & F_FREE) printf(" FREE");
		if(ins->phase[ins->phase_index].curr->flags & F_MARK) printf(" MARK");
		printf(" )");
	}
	printf("\n");
}
