#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

struct my_s{
	int fd;
	unsigned char *z;
	size_t maplen;
};

void *cbcreate(void){
	struct stat stat;
	struct my_s *r=malloc(sizeof(struct my_s));
	r->fd=open("inter/CNS11643", O_RDONLY);
	fstat(r->fd, &stat);
	r->maplen=stat.st_size;
	r->z=mmap(0,stat.st_size,PROT_READ, MAP_PRIVATE,r->fd,0);
	return r;
}

void cbdestroy(void *p){
	struct my_s *r=p;
	munmap(r->z, r->maplen);
	close(r->fd);
	free(p);
}

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	unsigned int len;
	struct state_s state;
	struct data_s *data_ptr;
	unsigned char *ptr;
	int i;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phasen];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	data=ins->phase[ins->phasen-1].data->data;

	switch(*data){
		case 0x01:
			memcpy(&state, t->z, sizeof(struct state_s));
			for(i=0;i<4;++i){
				memcpy(&state, t->z + (uintptr_t)state.sub[data[i]], sizeof(struct state_s));
				if(state.status==DEADEND){
					break;
				}
			}
			switch(state.status){
				case MATCH:
				case SUBMATCH:
					this_phase->state.status=NEXTPHASE;
					for(data_ptr=state.data;data_ptr;){
						this_phase->data_tail->next=malloc(sizeof(struct data_s));
						this_phase->data_tail=this_phase->data_tail->next;
						memcpy(this_phase->data_tail, (unsigned char *)(t->z+(uintptr_t)data_ptr), sizeof(struct data_s));
						data_ptr=this_phase->data_tail->next;
						this_phase->data_tail->next=NULL;
						ptr=(unsigned char *)(t->z+(uintptr_t)this_phase->data_tail->data);
						this_phase->data_tail->data=malloc(this_phase->data_tail->len);
						memcpy(this_phase->data_tail->data, ptr, this_phase->data_tail->len);
						this_phase->data_tail->data[0]=0;
					}
					return;
				default:
					this_phase->state.status=DEADEND;
					return;
			}
		case 0x02:
			len=ins->phase[ins->phasen-1].data->len-1;

			this_phase->data_tail->next=malloc(sizeof(struct data_s));
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			
			this_phase->data_tail->len=4;
			this_phase->data_tail->data=malloc(4);
			memcpy(this_phase->data_tail->data, data, this_phase->data_tail->len);
			this_phase->data_tail->data[0]=0;
			this_phase->state.status=NEXTPHASE;
			return;
		default:
			this_phase->state.status=DEADEND;
			return;
	}
}
