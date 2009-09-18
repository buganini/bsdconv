#include <stdlib.h>
#include "../../src/bsdconv.h"

#define bb00000111 0x07
#define bb10000000 0x80
#define bb11100000 0xE0
#define bb11000000 0xC0
#define bb11110000 0xF0
#define bb11111000 0XF8
#define bb00011100 0X1C
#define bb00111111 0X3F
#define bb00001111 0X0F
#define bb00000011 0X03
#define bb00111100 0X3C

struct my_s{
	int status;
	unsigned char buf[16];
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

#define CONTINUE() do{	\
	this_phase->state.status=CONTINUE;	\
	return;	\
}while(0);

#define DEADEND() do{	\
	this_phase->state.status=DEADEND;	\
	t->status=0;	\
	return;	\
}while(0);

#define PASS() do{	\
	this_phase->state.status=NEXTPHASE;	\
	t->status=0;	\
	return;	\
}while(0);

#define APPEND(n) do{	\
	this_phase->data_tail->next=malloc(sizeof(struct data_s));	\
	this_phase->data_tail=this_phase->data_tail->next;	\
	this_phase->data_tail->next=NULL;	\
	this_phase->data_tail->len=n;	\
	p=this_phase->data_tail->data=malloc(n);	\
	p[0]=0x01;	\
}while(0);

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[0];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	unsigned char d=*ins->from_data, *p;
	switch(t->status){
		case 0:
			if((d & bb10000000) == 0){
				DEADEND();
			}else if((d & bb11100000) == bb11000000){
				t->status=21;
				t->buf[0]=(d >> 2) & bb00000111;
				t->buf[1]=(d << 6) & bb11000000;
			}else if((d & bb11110000) == bb11100000){
				t->status=31;
				t->buf[0]=(d << 4) & bb11110000;
			}else if((d & bb11111000) == bb11110000){
				t->status=41;
				t->buf[0]=(d << 2) & bb00011100;
			}else{
				DEADEND();
			}
			CONTINUE();
			break;
		case 21:
			if((d & bb11000000) == bb10000000){
				t->buf[1] |= d & bb00111111;
				APPEND(3);
				p[1]=t->buf[0];
				p[2]=t->buf[1];
				PASS();
			}else{
				DEADEND();
			}
			break;
		case 31:
			if((d & bb11000000) == bb10000000){
				t->status=32;
				t->buf[0] |= (d >> 2) & bb00001111;
				t->buf[1]=(d << 6) & bb11000000;
				CONTINUE();
			}else{
				DEADEND();
			}
			break;
		case 32:
			if((d & bb11000000) == bb10000000){
				t->buf[1] |= d & bb00111111;
				APPEND(3);
				p[1]=t->buf[0];
				p[2]=t->buf[1];
				PASS();
			}else{
				DEADEND();
			}
			break;
		case 41:
			if((d & bb11000000) == bb10000000){
				t->status=42;
				t->buf[0] |= (d >> 4) & bb00000011;
				t->buf[1]=(d << 4) & bb11110000;
				CONTINUE();
			}else{
				DEADEND();
			}
			break;
		case 42:
			if((d & bb11000000) == bb10000000){
				t->status=43;
				t->buf[1] |= (d >> 2) & bb00001111;
				t->buf[2]=(d << 6) & bb11000000;
				CONTINUE();
			}else{
				DEADEND();
			}
			break;
		case 43:
			if((d & bb11000000) == bb10000000){
				t->buf[2] |= d & bb00111111;
				APPEND(4);
				p[1]=t->buf[0];
				p[2]=t->buf[1];
				p[3]=t->buf[2];
				PASS();
			}else{
				DEADEND();
			}
			break;
		default:
			DEADEND();
	}
}
