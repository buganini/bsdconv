#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	printf("asdfas\n");
	FILE *fp=stdout;
	while(arg){
		if(strcasecmp(arg->key, "STDERR")==0){
			fp=stderr;
		}else if(strcasecmp(arg->key, "STDOUT")==0){
			fp=stdout;
		}else{
			return EINVAL;
		}
		arg=arg->next;
	}
	THIS_CODEC(ins)->priv=fp;
	return 0;
}

void cbconv(struct bsdconv_instance *ins){
	FILE *fp=THIS_CODEC(ins)->priv;
	int i;
	ins->phase[ins->phase_index].state.status=NEXTPHASE;

	for(i=0;i<ins->phase[ins->phase_index].curr->len;++i){
		fprintf(fp, "%02X",UCP(ins->phase[ins->phase_index].curr->data)[i]);
	}
	if(ins->phase[ins->phase_index].curr->flags){
		fprintf(fp, " (");
		if(ins->phase[ins->phase_index].curr->flags & F_FREE) fprintf(fp, " FREE");
		if(ins->phase[ins->phase_index].curr->flags & F_MARK) fprintf(fp, " MARK");
		fprintf(fp, " )");
	}
	fprintf(fp, "\n");
}
