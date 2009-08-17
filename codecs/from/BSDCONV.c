#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

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
	t->flag=0;

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
		t->flag=1;
	}else{
		if(t->flag){
			t->flag=0;
			t->data.len=0;
		}
		ins->from_state.status=SUBMATCH;
		ins->from_state.data=&(t->data);
		if(t->data.len >= t->size){
			t->size+=8;
			t->data.data=realloc(t->data.data,t->size);
		}
		t->data.data[t->data.len]=d;
		t->data.len+=1;
	}
}
