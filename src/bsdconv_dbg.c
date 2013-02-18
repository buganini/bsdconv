/*
 * Copyright (c) 2011-2013 Kuan-Chung Chiu <buganini@gmail.com>
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
#include <string.h>
#include "bsdconv.h"

extern int loadcodec(struct bsdconv_codec_t *cd, int type);
extern void unloadcodec(struct bsdconv_codec_t *cd);

void print_state(struct state_rt *state);
void print_data(uintptr_t val);

struct bsdconv_codec_t cd;

void print_state(struct state_rt *state){
	const char *s=NULL;
	switch(state->status){
		case CONTINUE: s="CONTINUE"; break;
		case DEADEND: s="DEADEND"; break;
		case MATCH: s="MATCH"; break;
		case SUBMATCH: s="SUBMATCH"; break;
		case SUBROUTINE: s="SUBROUTINE"; break;
		case SUBMATCH_SUBROUTINE: s="SUBMATCH_SUBROUTINE"; break;
		case NEXTPHASE: s="NEXTPHASE"; break;
		case PASSTHRU: s="PASSTHRU"; break;
		case NOMATCH: s="NOMATCH"; break;
		case NOOP: s="NOOP"; break;
	}
	printf("State: %s @ %p\n", s, state);
	printf("Data:");
	print_data((uintptr_t)state->data);
}

void print_data(uintptr_t val){
	struct data_rt data;
	int i;
	if(val==0)
		printf(" 0x0");
	else while(val){
		memcpy(&data, (char *)(cd.z+val), sizeof(struct data_st));
		data.data=(cd.z+(uintptr_t)data.data);
		for(i=0;i<data.len;++i){
			printf("%02X", UCP(data.data)[i]);
		}
		printf("\n");
		val=(uintptr_t)data.next;
	}
	printf("\n");
}

int main(int argc, char *argv[]){
	if(argc!=3)
		return 1;

	int type;

	if(strcmp(argv[1], "FROM")==0){
		type=FROM;
	}else if(strcmp(argv[1], "INTER")==0){
		type=INTER;
	}else if(strcmp(argv[1], "TO")==0){
		type=TO;
	}else{
		return 1;
	}

	cd.desc=argv[2];
	if(!loadcodec(&cd, type)){
		fprintf(stderr, "Failed opening codec %s.\n", argv[2]);
		exit(0);
	}

	struct state_rt state;
	offset_t offset;
	char op[16];
	unsigned char c;
	uintptr_t val;
	memcpy(&state, cd.z, sizeof(struct state_rt));
	while(!feof(stdin)){
		printf("bsdconv_dbg> ");
		scanf("%s %p", op, (void **) &val);
		if(strcmp(op, "input")==0){
			c=val;
			if(c>=state.beg && c<state.end)
				memcpy(&offset, cd.z + (uintptr_t)state.base + (c - state.beg) * sizeof(offset_t), sizeof(offset_t));
			else
				offset=0;
			memcpy(&state, cd.z + offset, sizeof(struct state_st));
			print_state(&state);
		}else if(strcmp(op, "data")==0){
			print_data(val);
		}else if(strcmp(op, "reset")==0){
			memcpy(&state, cd.z, sizeof(struct state_rt));
		}else{
			break;
		}
	}

	unloadcodec(&cd);

	return 0;
}
