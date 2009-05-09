#include <stdlib.h>
#include "../../src/bsdconv.h"

#define bb10000000 0x80
#define bb11000000 0xC0
#define bb00000011 0x03
#define bb00111111 0x3f
#define bb11111000 0xf8
#define bb00011100 0x1c
#define bb11100000 0xe0
#define bb00001111 0x0f
#define bb00111100 0x3c

void callback(struct bsdconv_instruction *ins){
	unsigned char *data, *p;
	unsigned int len;
	data=ins->to_data->data;
	if(*data!=0x01){
		ins->to_state.status=DEADEND;
		return;
	}
	data+=1;
	len=ins->to_data->len-1;

	ins->out_data_tail->next=malloc(sizeof(struct data_s));
	ins->out_data_tail=ins->out_data_tail->next;
	ins->out_data_tail->next=NULL;

	switch(len){
		case 1:
			switch(*data & bb10000000){
				case 0:
					ins->out_data_tail->len=1;
					ins->out_data_tail->data=malloc(1);
					*(ins->out_data_tail->data)=*data;
					break;
				default:
					ins->out_data_tail->len=2;
					ins->out_data_tail->data=malloc(2);
					p=ins->out_data_tail->data;
					*p=bb11000000;
					*p |= (*data >> 6) & bb00000011;
					p++;
					*p=bb10000000;
					*p |= *data & bb00111111;
					break;
			}
			break;
		case 2:
			switch(*data & bb11111000){
				case 0:
					ins->out_data_tail->len=2;
					ins->out_data_tail->data=malloc(2);
					p=ins->out_data_tail->data;
					*p=bb11000000;
					*p |= (*data << 2) & bb00011100;;
					data++;
					*p |= (*data >> 6) & bb00000011;
					p++;
					*p=bb10000000;
					*p |= *data & bb00111111;
					break;
				default:
					ins->out_data_tail->len=3;
					ins->out_data_tail->data=malloc(3);
					p=ins->out_data_tail->data;
					*p=bb11100000;
					*p |= *data & bb00001111;
					p++;
					*p=bb10000000;
					*p |= (*data << 2) & bb00111100;
					data++;
					*p |= (*data >> 6) & bb00000011;
					p++;
					*p=bb10000000;
					*p |= *data & bb00111111;
					break;
			}
			break;
	}
}
