#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
	int *tbl;
	int b;
	union {
		unsigned char c[4];
		uint32_t i;
	} buf;
};

void *cbinit(void){
	struct my_s *ret;
	ret=malloc(sizeof(struct my_s));
	ret->status=0;
	return ret;
}

void cbclear(void *p){
	free(p);
}

#define CONTINUE() do{	\
	ins->from_state.status=CONTINUE;	\
	return;	\
}while(0);

#define DEADEND() do{	\
	ins->from_state.status=DEADEND;	\
	t->status=0;	\
	return;	\
}while(0);

#define APPEND(n) do{	\
	ins->inter_data_tail->next=malloc(sizeof(struct data_s));	\
	ins->inter_data_tail=ins->inter_data_tail->next;	\
	ins->inter_data_tail->next=NULL;	\
	ins->inter_data_tail->len=n;	\
	p=ins->inter_data_tail->data=malloc(n);	\
	p[0]=0x01;	\
}while(0);

int dec[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int hex[256]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void callback(struct bsdconv_instruction *ins){
	unsigned char ob[8];
	int i,j=0;
	struct my_s *t=ins->fpriv[ins->from_index];
	unsigned char d=*ins->from_data, *p;
	if(d==';' && t->status){
		//put data
		t->buf.i=htonl(t->buf.i);
		for(i=0;i<4;i++){
			if(t->buf.c[i] || j)
				ob[j++]=t->buf.c[i];
		}
		ins->inter_data_tail->next=malloc(sizeof(struct data_s));
		ins->inter_data_tail=ins->inter_data_tail->next;
		ins->inter_data_tail->next=NULL;
		ins->inter_data_tail->len=j+1;
		p=ins->inter_data_tail->data=malloc(j+1);
		p[0]=0x01;
		memcpy(&p[1], ob, j);
		ins->from_state.status=NEXTPHASE;
		t->status=0;
		return;
	}
	if(t->status){
		++t->status;
		if(t->tbl[d]==-1) DEADEND();
		t->buf.i*=t->b;
		t->buf.i+=t->tbl[d];
	}else{
		if(d=='x'){
			t->status=1000;
			t->tbl=hex;
			t->b=16;
			t->buf.i=0;
			CONTINUE();
		}
		t->b=10;
		t->tbl=dec;
		if(t->tbl[d]==-1) DEADEND();
		t->buf.i=t->tbl[d];
		t->status=1;
	}
	CONTINUE();
}
