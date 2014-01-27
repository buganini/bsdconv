#include <stdio.h>
#include <string.h>
#include "bsdconv.h"

extern int loadcodec(struct bsdconv_codec *cd, int type);
extern void unloadcodec(struct bsdconv_codec *cd);

void print_state(struct state_rt *state);
void print_data(uintptr_t val);

struct bsdconv_codec cd;

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
		case NOMATCH: s="NOMATCH"; break;
		case NOOP: s="NOOP"; break;
	}
	printf("State: %s\n", s);
	printf("Beg: 0x%02X\n", (int)state->beg);
	printf("End: 0x%02X\n", (int)state->end);
	printf("Data:");
	print_data((uintptr_t)de_offset(state->data));
}

void print_data(uintptr_t val){
	struct data_st data;
	unsigned char *c;
	int i;
	if(val==0)
		printf(" 0x0");
	else while(val){
		memcpy(&data, (char *)(cd.z+de_offset(val)), sizeof(struct data_st));
		c=UCP(cd.z+(uintptr_t)data.data);
		for(i=0;i<data.len;++i){
			printf("%02X", c[i]);
		}
		printf("\n");
		val=(uintptr_t)de_offset(data.next);
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

	struct state_st dstate;
	struct state_rt state;
	offset_t offset=0;
	char op[16];
	unsigned int c;
	uintptr_t val;
	memcpy(&dstate, cd.z, sizeof(struct state_st));
	state.status=dstate.status;
	state.data=(void *)(uintptr_t)de_offset(dstate.data);
	state.beg=dstate.beg;
	state.end=dstate.end;
	state.base=de_offset(dstate.base);
	while(!feof(stdin)){
		printf("bsdconv_dbg@%x> ", offset);
		scanf("%s %p", op, (void **) &val);
		if(strcmp(op, "input")==0){
			c=val;
			if(c>=state.beg && c<state.end)
				memcpy(&offset, cd.z + (uintptr_t)state.base + (c - state.beg) * sizeof(offset_t), sizeof(offset_t));
			else
				offset=0;
			memcpy(&dstate, cd.z + offset, sizeof(struct state_st));
			state.status=dstate.status;
			state.data=(void *)(uintptr_t)de_offset(dstate.data);
			state.beg=dstate.beg;
			state.end=dstate.end;
			state.base=de_offset(dstate.base);
			print_state(&state);
		}else if(strcmp(op, "data")==0){
			print_data(val);
		}else if(strcmp(op, "reset")==0){
			memcpy(&dstate, cd.z, sizeof(struct state_st));
			state.status=dstate.status;
			state.data=(void *)(uintptr_t)de_offset(dstate.data);
			state.beg=dstate.beg;
			state.end=dstate.end;
			state.base=de_offset(dstate.base);
		}else{
			break;
		}
	}

	unloadcodec(&cd);

	return 0;
}
