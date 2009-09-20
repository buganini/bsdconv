#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

void *cbcreate(void){
	struct bsdconv_codec_t *cd=malloc(sizeof(struct bsdconv_codec_t));
	if(!loadcodec(cd, "inter/CNS11643", 1)){
		free(cd);
		return NULL;
	}
	return cd;
}

void cbdestroy(void *p){
	struct bsdconv_codec_t *cd=p;
	unloadcodec(cd);
	free(p);
}

void callback(struct bsdconv_instance *ins){
	unsigned char *data, *p, buf[128]={0};
	unsigned int len, i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phasen-1];
	data=prev_phase->data->data;
	struct state_s state;
	struct data_s *data_ptr, *orig_next, *my_tail;
	unsigned char *ptr;
	struct bsdconv_codec_t *t=this_phase->codec[this_phase->index].priv;
	switch(*data){
		case 0x01:
			memcpy(&state, t->z, sizeof(struct state_s));
			for(i=0;i<prev_phase->data->len;++i){
				memcpy(&state, t->z + (uintptr_t)state.sub[data[i]], sizeof(struct state_s));
				if(state.status==DEADEND){
					break;
				}
			}
			switch(state.status){
				case MATCH:
				case SUBMATCH:
					orig_next=prev_phase->data->next;
					free(data);
					my_tail=prev_phase->data;
					memcpy(my_tail, (unsigned char *)(t->z+(uintptr_t)state.data), sizeof(struct data_s));
					ptr=(unsigned char *)(t->z+(uintptr_t)my_tail->data);
					my_tail->data=malloc(my_tail->len);
					memcpy(my_tail->data, ptr, my_tail->len);
					data_ptr=my_tail->next;
					my_tail->next=NULL;
					while(data_ptr){
						my_tail->next=malloc(sizeof(struct data_s));
						my_tail=my_tail->next;
						memcpy(my_tail, (unsigned char *)(t->z+(uintptr_t)data_ptr), sizeof(struct data_s));
						data_ptr=my_tail->next;
						my_tail->next=orig_next;
						ptr=(unsigned char *)(t->z+(uintptr_t)my_tail->data);
						my_tail->data=malloc(my_tail->len);
						memcpy(my_tail->data, ptr, my_tail->len);
					}
					data=prev_phase->data->data;
					break;
			}
			break;
	}
	if(*data!=0x02){
		this_phase->state.status=DEADEND;
		return;
	}
	this_phase->state.status=NEXTPHASE;
	p=buf;
	i=*data;
	data+=1;
	len=prev_phase->data->len-1;
	this_phase->data_tail->next=malloc(sizeof(struct data_s));
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	sprintf(p,"<img class=\"cns11643_img\" src=\"http://www.cns11643.gov.tw/AIDB/png.do?page=");
	TAILIZE(p);
	sprintf(p,"%X", data[0]);
	TAILIZE(p);
	sprintf(p,"&code=");
	for(i=1;i<len;i++){
		TAILIZE(p);
		sprintf(p,"%02X", data[i]);
	}
	TAILIZE(p);
	sprintf(p, "\" />");
	TAILIZE(p);
	len=p-buf;
	this_phase->data_tail->len=len;
	this_phase->data_tail->data=malloc(len);
	memcpy(this_phase->data_tail->data, buf, len);

	return;
}
