#include <stdio.h>
#include "../../src/bsdconv.h"

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=bsdconv_counter(ins, "SCORE");
	return 0;
}

void cbconv(struct bsdconv_instance *ins){
	struct data_rt *data_ptr;
	bsdconv_counter_t *counter=THIS_CODEC(ins)->priv;
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct data_st data_st;
	memcpy(&data_st, (char *)((this_phase->codec[this_phase->index].data_z)+(uintptr_t)this_phase->state.data), sizeof(struct data_st));
	data=UCP((THIS_CODEC(ins)->data_z)+(uintptr_t)de_offset(data_st.data));

	*counter += *data;

	LISTCPY_ST(ins, this_phase->data_tail, (void *)(uintptr_t)de_offset(data_st.next), THIS_CODEC(ins)->data_z);

	this_phase->state.status=NEXTPHASE;
	return;
}
