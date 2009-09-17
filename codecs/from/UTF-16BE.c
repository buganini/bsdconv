#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"


struct my_s{
	int status;
	unsigned char buf[4];
};

void *cbcreate(void){
	return  malloc(sizeof(struct my_s));
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->status=0;
}

void cbdestroy(void *p){
	free(p);
}

#define bb11111100 0xFC
#define bb11011000 0xD8
#define bb11011100 0xDC
#define bb00000011 0x03

#define CONTINUE() do{	\
	this_phase->state.status=CONTINUE;	\
	return;	\
}while(0);

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[0];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	unsigned char d=*ins->from_data, buf[3]={0};
	int i;
	size_t l;
	switch(t->status){
		case 0:
			t->buf[0]=d;
			t->status=1;
			CONTINUE();
			break;
		case 1:
			t->buf[1]=d;
			if((t->buf[0] & bb11111100) == bb11011000){
				t->status=2;
				CONTINUE();
			}else{
				t->status=0;
				for(i=0;i<2;++i){
					if(t->buf[i]) break;
				}
				l=(2-i)+1;
				this_phase->data_tail->next=malloc(sizeof(struct data_s));
				this_phase->data_tail=this_phase->data_tail->next;
				this_phase->data_tail->next=NULL;
				this_phase->data_tail->len=l;
				this_phase->data_tail->data=malloc(l);
				this_phase->data_tail->data[0]=0x01;
				memcpy(&this_phase->data_tail->data[1], &t->buf[i], l-1);
				this_phase->state.status=NEXTPHASE;
			}
			break;
		case 2:
			t->buf[2]=d;
			t->status=3;
			CONTINUE();
			break;
		case 3:
			t->buf[3]=d;
			t->status=0;
			if((t->buf[2] & bb11111100) == bb11011100){
				buf[0]=(t->buf[0] & bb00000011) << 2;
				buf[0] |= (t->buf[1] >> 6) & bb00000011;
				buf[0] += 1;
				buf[1]=(t->buf[1] << 2) & bb11111100;
				buf[1] |= t->buf[2] & bb00000011;
				buf[2]=t->buf[3];
				for(i=0;i<3;++i){
					if(buf[i]) break;
				}
				l=(3-i)+1;
				this_phase->data_tail->next=malloc(sizeof(struct data_s));
				this_phase->data_tail=this_phase->data_tail->next;
				this_phase->data_tail->next=NULL;
				this_phase->data_tail->len=l;
				this_phase->data_tail->data=malloc(l);
				this_phase->data_tail->data[0]=0x01;
				memcpy(&this_phase->data_tail->data[1], &buf[i], l-1);
				this_phase->state.status=NEXTPHASE;
			}else{
				this_phase->state.status=DEADEND;
				return;
			}
			break;
	}
}