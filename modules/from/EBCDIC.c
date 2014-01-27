#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->status=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	struct data_st data;
	unsigned char *c;

	memcpy(&data, (char *)(this_phase->codec[this_phase->index].data_z+(uintptr_t)this_phase->state.data), sizeof(struct data_st));
	c=UCP(this_phase->codec[this_phase->index].data_z+de_offset(data.data));

	if(data.len==2 && c[0]=='\x01'){
		if(c[1]=='\x0E'){
			t->status=1;
			this_phase->state.status=NEXTPHASE;
			return;
		}else if(c[1]=='\x0F'){
			t->status=0;
			this_phase->state.status=NEXTPHASE;
			return;
		}
	}

	if(t->status==0){
		this_phase->state.status=MATCH;
	}else{
		this_phase->state.status=SUBMATCH;
	}

	return;
}
