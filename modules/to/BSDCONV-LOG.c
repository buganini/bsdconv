#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=fopen(getenv("BSDCONV_TO_LOG"),"a");
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	void *p=THIS_CODEC(ins)->priv;
	fclose(p);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	FILE *fp=THIS_CODEC(ins)->priv;
	int i;
	this_phase->state.status=NEXTPHASE;

	for(i=0;i<this_phase->curr->len;++i){
		fprintf(fp,"%02X",UCP(this_phase->curr->data)[i]);
	}
	if(this_phase->curr->flags){
		fprintf(fp," (");
		if(this_phase->curr->flags & F_FREE) fprintf(fp, " FREE");
		if(this_phase->curr->flags & F_MARK) fprintf(fp, " MARK");
		fprintf(fp," )");
	}
	fprintf(fp,"\n");
	fflush(fp);
}
