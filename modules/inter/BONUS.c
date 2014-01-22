/*
 * Copyright (c) 2014 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

	LISTCPY_ST(this_phase->data_tail, (void *)(uintptr_t)de_offset(data_st.next), THIS_CODEC(ins)->data_z);

	this_phase->state.status=NEXTPHASE;
	return;
}
