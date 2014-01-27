#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=fopen(getenv("BSDCONV_FROM_LOG"),"a");
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	void *fp=THIS_CODEC(ins)->priv;
	fclose(fp);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	FILE *fp=this_phase->codec[this_phase->index].priv;
	fprintf(fp,"%02X\n", (int)UCP(this_phase->curr->data)[this_phase->i]);
	this_phase->state.status=NEXTPHASE;
	fflush(fp);
}
