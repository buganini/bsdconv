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

void cbinit(struct bsdconv_codec_t *cdc, struct data_s *r){
	cdc->data_z=0;
	r->data.size=0;
	r->data.data=NULL;
	r->data.next=0;
	r->len=0;
	r->flag=0;

}

void cbclear(void *p){
	free(p);
}

int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void callback(struct bsdconv_instance *ins){
	unsigned char ob[8];
	int i,j=0;
	struct my_s *t=ins->from_priv[ins->from_index];
	unsigned char d=*ins->from_data, *p;
	if(hex[d]==-1){
		ins->from_state.status=DEADEND;
		t->flag=1;
	}else{
		it(t->flag){
			t->flag=0;
			t->len=0;
		}
		ins->from_state.status=SUBMATCH;
		ins->from_state.data=&(t->data);
		if(t->len >= t->size){
			t->size+=8;
			t->data=realloc(t->data,t->size);
		}
		t->data[t->len]=d;
		t->len+=1;
	}
}
