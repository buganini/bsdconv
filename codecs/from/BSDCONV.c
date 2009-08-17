#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define F_CLEAR 0
#define F_A 1
#define F_B 2

struct my_s {
	struct data_s data;
	/* extend struct data_s */
	size_t size;
	unsigned char flag;
};

void *cbcreate(void){
	return malloc(sizeof(struct my_s));
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *t){
	cdc->data_z=0;
	t->data.len=0;
	t->data.data=NULL;
	t->data.next=0;
	t->size=0;
	t->flag=F_A;

}

void cbclear(void *p){
	free(p);
}

int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void callback(struct bsdconv_instance *ins){
	struct my_s *t=ins->from_priv[ins->from_index];
	unsigned char d=*ins->from_data;
	if(hex[d]==-1){
		ins->from_state.status=DEADEND;
		t->flag=F_CLEAR;
	}else{
		if(t->flag==F_CLEAR){
			t->flag=F_A;
			t->data.len=0;
		}
		ins->from_state.status=SUBMATCH;
		ins->from_state.data=&(t->data);
		switch(t->flag){
			case F_A:
				if(t->data.len >= t->size){
					t->size+=8;
					t->data.data=realloc(t->data.data,t->size);
				}
				t->data.data[t->data.len]=hex[d];
				t->data.len+=1;
				t->flag=F_B;
				break;
			case F_B:
				t->data.data[t->data.len-1]<<=4;
				t->data.data[t->data.len-1]|=hex[d];
				t->flag=F_A;
				break;
		}
	}
}
