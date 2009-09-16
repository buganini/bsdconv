#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define bb11011000 0xD8
#define bb00000011 0x03
#define bb11000000 0xC0
#define bb00111111 0x3F
#define bb11011100 0xDC

#define SWAP(a,b,i) ((i)=(a), (a)=(b), (b)=(i))

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p, c;
	unsigned int len, i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phasen-1];
	data=prev_phase->data->data;
	if(*data!=0x01){
		this_phase->state.status=DEADEND;
		return;
	}
	if(prev_phase->data->len > 3){
		this_phase->state.status=NEXTPHASE;
		data+=1;

		this_phase->data_tail->next=malloc(sizeof(struct data_s));
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=4;
		p=this_phase->data_tail->data=malloc(4);

		c=*data-1;
		*p=bb11011000;
		*p |= (c >> 2) & bb00000011;
		++p;
		*p=(c << 6) & bb11000000;
		++data;
		*p |= (*data >> 2) & bb00111111;
		++p;
		*p=bb11011100;
		*p |= *data & bb00000011;
		++p;
		++data;
		*p=*data;

		data=this_phase->data_tail->data;

		SWAP(data[0],data[1],i);
		SWAP(data[2],data[3],i);
	}else{
		this_phase->state.status=NEXTPHASE;
		data+=1;
		len=prev_phase->data->len-1;

		this_phase->data_tail->next=malloc(sizeof(struct data_s));
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=2;
		this_phase->data_tail->data=malloc(2);
		for(i=0;i<2-len;++i){
			this_phase->data_tail->data[i]=0x0;
		}
		memcpy(&this_phase->data_tail->data[i], data, len);
		data=this_phase->data_tail->data;

		SWAP(data[0],data[1],i);
	}
	return;
}
