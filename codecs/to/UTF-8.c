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
#define bb11110000 0xf0
#define bb00000111 0x07
#define bb00110000 0x30

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p;
	unsigned int len;
	data=ins->phase[ins->phasen-1].data->data;

	ins->phase[ins->phasen].state.status=NEXTPHASE;
	data+=1;
	len=ins->phase[ins->phasen-1].data->len-1;

	if(len==1 && (*data & bb10000000)==0){
		ins->phase[ins->phasen].state.status=DEADEND;
		return;
	}

	ins->phase[ins->phasen].data_tail->next=malloc(sizeof(struct data_s));
	ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_tail->next;
	ins->phase[ins->phasen].data_tail->next=NULL;

	switch(len){
		case 1:
			ins->phase[ins->phasen].data_tail->len=2;
			ins->phase[ins->phasen].data_tail->data=malloc(2);
			p=ins->phase[ins->phasen].data_tail->data;
			*p=bb11000000;
			*p |= (*data >> 6) & bb00000011;
			++p;
			*p=bb10000000;
			*p |= *data & bb00111111;
			break;
		case 2:
			switch(*data & bb11111000){
				case 0:
					ins->phase[ins->phasen].data_tail->len=2;
					ins->phase[ins->phasen].data_tail->data=malloc(2);
					p=ins->phase[ins->phasen].data_tail->data;
					*p=bb11000000;
					*p |= (*data << 2) & bb00011100;;
					++data;
					*p |= (*data >> 6) & bb00000011;
					++p;
					*p=bb10000000;
					*p |= *data & bb00111111;
					break;
				default:
					ins->phase[ins->phasen].data_tail->len=3;
					ins->phase[ins->phasen].data_tail->data=malloc(3);
					p=ins->phase[ins->phasen].data_tail->data;
					*p=bb11100000;
					*p |= (*data >> 4) & bb00001111;
					++p;
					*p=bb10000000;
					*p |= (*data << 2) & bb00111100;
					++data;
					*p |= (*data >> 6) & bb00000011;
					++p;
					*p=bb10000000;
					*p |= *data & bb00111111;
					break;
			}
			break;
		case 3:
			switch(*data & bb11100000){
				case 0:
					ins->phase[ins->phasen].data_tail->len=4;
					ins->phase[ins->phasen].data_tail->data=malloc(4);
					p=ins->phase[ins->phasen].data_tail->data;
					*p=bb11110000;
					*p |= (*data >> 2) & bb00000111;
					++p;
					*p=bb10000000;
					*p |= (*data << 4) & bb00110000;
					++data;
					*p |= (*data >> 4) & bb00001111;
					++p;
					*p=bb10000000;
					*p |= (*data << 2) & bb00111100;
					++data;
					*p |= (*data >> 6) & bb00000011;
					++p;
					*p=bb10000000;
					*p |= *data & bb00111111;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}
