/*
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void * cbcreate(void){
	return fopen(getenv("BSDCONV_TO_LOG"),"a");
}

void cbdestroy(FILE *fp){
	fclose(fp);
}

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	FILE *fp=this_phase->codec[this_phase->index].priv;
	int i;
	ins->phase[ins->phase_index].state.status=NEXTPHASE;

	for(i=0;i<ins->phase[ins->phase_index].data->len;++i){
		fprintf(fp,"%02X",UCP(ins->phase[ins->phase_index].data->data)[i]);
	}
	if(ins->phase[ins->phase_index].data->flags){
		fprintf(fp," (");
		if(ins->phase[ins->phase_index].data->flags & F_FREE) printf(" FREE");
		if(ins->phase[ins->phase_index].data->flags & F_SKIP) printf(" SKIP");
		fprintf(fp," )");
	}
	fprintf(fp,"\n");
	fflush(fp);
}
