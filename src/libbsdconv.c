#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "bsdconv.h"

#define RESET(X) do{	\
	ins->phase[X].index=0;	\
	memcpy(&ins->phase[X].state, ins->phase[X].codec[ins->phase[X].index].z, sizeof(struct state_s));	\
}while(0);

void bsdconv_init(struct bsdconv_instance *ins){
	int i, j;
	switch(ins->mode){
		case BSDCONV_BB:
			ins->feed=ins->in_buf;
			ins->feed_len=ins->in_len;
			ins->back=ins->out_buf;
			break;
		case BSDCONV_CB:
			ins->back=ins->out_buf;
			break;
		case BSDCONV_BC:
			ins->feed=ins->in_buf;
			ins->feed_len=ins->in_len;
			break;
		case BSDCONV_BM:
			ins->feed=ins->in_buf;
			ins->feed_len=ins->in_len;
			ins->back=NULL;
			break;
		case BSDCONV_CC:
			break;
		case BSDCONV_CM:
			ins->back=NULL;
			break;
	}

	ins->ierr=0;
	ins->oerr=0;

	ins->from_bak=ins->from_data=ins->feed;
	
	for(i=0;i<=ins->phasen;i++){
		RESET(i)
		ins->phase[i].data_head->next=NULL;
		ins->phase[i].bak=ins->phase[i].data=ins->phase[i].data_tail=ins->phase[i].data_head;
		ins->phase[i].match=NULL;
		for(j=0;j<=ins->phase[i].codecn;j++){
			if(ins->phase[i].codec[j].cbinit)
				ins->phase[i].codec[j].cbinit(&(ins->phase[i].codec[j]),ins->phase[i].codec[j].priv);
		}
	}
}

struct bsdconv_instance *bsdconv_create(const char *conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	struct stat stat;
	char *t;
	int i, j, brk;
	char buf[64], path[512];

	i=1;
	for(t=(char *)conversion;*t;t++){
		if(*t==':')++i;
	}
	if(i<2){
		errno=EINVAL;
		return NULL;
	}

	ins->phasen=i-1; //i is real length, but we use i-1 for a convient to use array boundary here
	ins->phase=malloc(sizeof(struct bsdconv_phase) * i);
	char *opipe[i];
	char *pipe[i];
	int npipe[i];

	t=strdup(conversion);

	for(i=0;i<=ins->phasen;++i){
		opipe[i]=(char *)strsep(&t, ":");
	}
	for(i=0;i<=ins->phasen;++i){
		if(*opipe[i]){
			npipe[i]=1;
			for(t=(char *)opipe[i];*t;t++){
				if(*t==','){
					npipe[i]++;
				}
			}
		}else{
			npipe[i]=0;
		}
		ins->phase[i].codecn=npipe[i];
		if(npipe[i]==0){
			errno=EINVAL;
			return NULL;
		}
	}

	chdir(PREFIX "/share/bsdconv");

	for(i=0;i<=ins->phasen;++i){
		pipe[i]=strdup(opipe[i]);
		ins->phase[i].codec=malloc(npipe[i] * sizeof(struct bsdconv_codec_t));
		ins->phase[i].codec[0].desc=pipe[i];
		if(i==0){
			chdir("from");
		}else if(i==ins->phasen){
			chdir("to");
		}else{
			chdir("inter");
		}
		brk=0;
		for(j=0,t=pipe[i];;++t){
			if(*t==',' || *t==0){
				if(*t==0){
					brk=1;
				}
				*t=0;
				strcpy(buf, ins->phase[i].codec[j].desc);
				realpath(buf, path);
				if((ins->phase[i].codec[j].fd=open(path, O_RDONLY))==-1){
					errno=EOPNOTSUPP;
					return NULL;
				}
				fstat(ins->phase[i].codec[j].fd, &stat);
				ins->phase[i].codec[j].maplen=stat.st_size;
				if((ins->phase[i].codec[j].data_z=ins->phase[i].codec[j].z=mmap(0,stat.st_size,PROT_READ, MAP_PRIVATE,ins->phase[i].codec[j].fd,0))==MAP_FAILED){
					errno=ENOMEM;
					return NULL;
				}
				strcat(path, ".so");
				ins->phase[i].codec[j].cbcreate=NULL;
				ins->phase[i].codec[j].cbinit=NULL;
				ins->phase[i].codec[j].callback=NULL;
				ins->phase[i].codec[j].cbdestroy=NULL;
				if((ins->phase[i].codec[j].dl=dlopen(path, RTLD_LAZY))){
					ins->phase[i].codec[j].callback=dlsym(ins->phase[i].codec[j].dl,"callback");
					ins->phase[i].codec[j].cbcreate=dlsym(ins->phase[i].codec[j].dl,"cbcreate");
					ins->phase[i].codec[j].cbinit=dlsym(ins->phase[i].codec[j].dl,"cbinit");
					ins->phase[i].codec[j].cbdestroy=dlsym(ins->phase[i].codec[j].dl,"cbdestroy");
				}
				if(j+1 < ins->phase[i].codecn){
					ins->phase[i].codec[++j].desc=t+1;
				}
				if(brk){
					break;
				}
			}else{
				*t=toupper(*t);
			}
		}
		chdir("..");

		ins->phase[i].codecn--;
		ins->phase[i].data_head=malloc(sizeof(struct data_s));
		for(j=0;j<=ins->phase[i].codecn;j++){
			if(ins->phase[i].codec[j].cbcreate){
				ins->phase[i].codec[j].priv=ins->phase[i].codec[j].cbcreate();
			}
		}
	}

	return ins;
}

void bsdconv_destroy(struct bsdconv_instance *ins){
	int i,j;
	struct data_s *data_ptr;

	for(i=0;i<=ins->phasen;i++){
		free(ins->phase[i].codec[0].desc);
		for(j=0;j<=ins->phase[i].codecn;j++){
			if(ins->phase[i].codec[j].cbdestroy){
				ins->phase[i].codec[j].cbdestroy(ins->phase[i].codec[j].priv);
			}
			if(ins->phase[i].codec[j].dl){
				dlclose(ins->phase[i].codec[j].dl);
			}
			munmap(ins->phase[i].codec[j].z, ins->phase[i].codec[j].maplen);
			close(ins->phase[i].codec[j].fd);
		}
		while(ins->phase[i].data_head){
			data_ptr=ins->phase[i].data_head;
			ins->phase[i].data_head=ins->phase[i].data_head->next;
			free(data_ptr);
		}
	}
	free(ins);
}

#define check_leftovers() do{	\
	for(phase_index=ins->phasen-1;phase_index>=0;--phase_index){	\
		if(ins->phase[phase_index].data->next){	\
			if(phase_index==ins->phasen-1){	\
				goto phase_to;	\
			}else{	\
				phase_index++;	\
				goto phase_inter;	\
			}	\
		}	\
	}	\
	if(ins->from_data < ins->feed+ins->feed_len) goto phase_from;	\
}while(0);

#define check_pending() do{	\
	for(phase_index=0;phase_index<=ins->phasen;++phase_index){	\
		if(ins->phase[phase_index].pend){	\
			if(phase_index==0){	\
				goto pass_to_inter;	\
			}else if(phase_index==ins->phasen){	\
				goto pass_to_to;	\
			}else{	\
				goto pass_to_out;	\
			}	\
		}	\
	}	\
}while(0);

int bsdconv(struct bsdconv_instance *ins){
	uintptr_t i;
	int phase_index;
	struct data_s *data_ptr;
	unsigned char *ptr;

	switch(ins->mode){
		case BSDCONV_BB:
			ins->back_len=0;
			if(ins->phase[ins->phasen].data_head->next) goto bb_out;
			check_leftovers();
			break;
		case BSDCONV_BC:
			check_leftovers();
			break;
		case BSDCONV_CB:
			ins->back_len=0;
			if(ins->phase[ins->phasen].data_head->next) goto cb_out;
			break;
		case BSDCONV_CC:
			break;
		case BSDCONV_BM:
			if(ins->back){
				ptr=ins->back;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					free(data_ptr);
				}
				return 0;
			}else{
				for(phase_index=ins->phasen-1;phase_index>=0;--phase_index){
					if(ins->phase[phase_index].data->next){
						if(phase_index==ins->phasen-1){
							goto phase_to;
						}else{
							phase_index++;
							goto phase_inter;
						}
					}
				}
			}
			break;
		case BSDCONV_CM:
			if(ins->back){
				ptr=ins->back;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					free(data_ptr);
				}
				return 0;
			}
			break;
		}

	//from
	phase_from:
	while(ins->from_data < ins->feed+ins->feed_len){
		memcpy(&ins->phase[0].state, ins->phase[0].codec[ins->phase[0].index].z + (uintptr_t)ins->phase[0].state.sub[*ins->from_data], sizeof(struct state_s));
		from_x:
		switch(ins->phase[0].state.status){
			case DEADEND:
				pass_to_inter:
				ins->phase[0].pend=0;
				if(ins->phase[0].match){
					listcpy(0, ins->phase[0].match, ins->phase[0].codec[ins->phase[0].index].data_z);
					ins->phase[0].match=NULL;
					RESET(0);

					ins->from_data=ins->from_bak;
					phase_index=1;
					goto phase_inter;
				}else if(ins->phase[0].index < ins->phase[0].codecn){
					ins->phase[0].index++;
					memcpy(&ins->phase[0].state, ins->phase[0].codec[ins->phase[0].index].z, sizeof(struct state_s));

					ins->from_data=ins->from_bak;
					continue;
				}else{
					ins->ierr++;

					RESET(0);

					ins->from_data=ins->from_bak;
					++ins->from_data;
					ins->from_bak=ins->from_data;
					continue;
				}
				break;
			case MATCH:
				++ins->from_data;
				ins->from_bak=ins->from_data;
				listcpy(0, ins->phase[0].state.data, ins->phase[0].codec[ins->phase[0].index].data_z);
				ins->phase[0].pend=0;
				ins->phase[0].match=NULL;
				RESET(0);
				phase_index=1;
				goto phase_inter;
			case SUBMATCH:
				ins->phase[0].match=ins->phase[0].state.data;
				++ins->from_data;
				ins->from_bak=ins->from_data;
				ins->phase[0].pend=1;
				break;
			case CALLBACK:
				ins->phase[0].codec[ins->phase[0].index].callback(ins);
				goto from_x;
			case NEXTPHASE:
				RESET(0);
				++ins->from_data;
				ins->from_bak=ins->from_data;
				ins->phase[0].pend=0;
				phase_index=1;
				goto phase_inter;
				break;
			case CONTINUE:
				ins->phase[0].pend=1;
				++ins->from_data;
				break;
			default:
				++ins->from_data;
		}
	}
	phase_index=1;

	//inter
	phase_inter:
	if(phase_index==ins->phasen){
		goto phase_to;
	}
	while(ins->phase[phase_index-1].data->next){
		ins->phase[phase_index-1].data=ins->phase[phase_index-1].data->next;
		for(i=0;i<ins->phase[phase_index-1].data->len;i++){
			memcpy(&ins->phase[phase_index].state, ins->phase[phase_index].codec[ins->phase[phase_index].index].z + (uintptr_t)ins->phase[phase_index].state.sub[*(ins->phase[phase_index-1].data->data+i)], sizeof(struct state_s));
			if(ins->phase[phase_index].state.status==DEADEND){
				break;
			}
		}
		switch(ins->phase[phase_index].state.status){
			case DEADEND:
				pass_to_to:
				ins->phase[phase_index].pend=0;
				if(ins->phase[phase_index].match){
					listcpy(phase_index, ins->phase[phase_index].match, ins->phase[phase_index].codec[ins->phase[phase_index].index].data_z);
					ins->phase[phase_index].match=NULL;
					ins->phase[phase_index].bak=ins->phase[phase_index].bak->next;
					listfree(phase_index-1,ins->phase[phase_index].bak);
					ins->phase[phase_index-1].data=ins->phase[phase_index-1].data_head;

					RESET(phase_index);
					goto phase_inter;
				}else if(ins->phase[phase_index].index < ins->phase[phase_index].codecn){
					ins->phase[phase_index].index++;
					memcpy(&ins->phase[phase_index].state, ins->phase[phase_index].codec[ins->phase[phase_index].index].z, sizeof(struct state_s));
					ins->phase[phase_index-1].data=ins->phase[phase_index-1].data_head;
					continue;
				}else{
					data_ptr=ins->phase[phase_index-1].data_head->next;
					ins->phase[phase_index-1].data_head->next=ins->phase[phase_index-1].data_head->next->next;
					ins->phase[phase_index-1].data=ins->phase[phase_index-1].data_head;
					data_ptr->next=NULL;
					ins->phase[phase_index].data_tail->next=data_ptr;
					if(ins->phase[phase_index-1].data_tail==data_ptr){
						ins->phase[phase_index-1].data_tail=ins->phase[phase_index-1].data_head;
					}
					ins->phase[phase_index-1].data=ins->phase[phase_index-1].data_head;

					RESET(phase_index);

					++phase_index;
					goto phase_inter;
				}
				break;
			case MATCH:
				ins->phase[phase_index-1].data=ins->phase[phase_index-1].data->next;
				listcpy(phase_index, ins->phase[phase_index].state.data, ins->phase[phase_index].codec[ins->phase[phase_index].index].data_z);
				listfree(phase_index-1,ins->phase[phase_index-1].data);
				ins->phase[phase_index].pend=0;
				ins->phase[phase_index].match=NULL;

				RESET(phase_index);

				ins->phase[phase_index-1].data=ins->phase[phase_index-1].data_head;

				++phase_index;
				goto phase_inter;
			case SUBMATCH:
				ins->phase[phase_index].match=ins->phase[phase_index].state.data;
				ins->phase[phase_index].bak=ins->phase[phase_index-1].data;
				ins->phase[phase_index].pend=1;
				break;
			case NEXTPHASE:
				++phase_index;
				goto phase_inter;
				break;
			case CONTINUE:
				ins->phase[phase_index].pend=1;
				break;
		}
		memcpy(&ins->phase[phase_index].state, ins->phase[phase_index].codec[ins->phase[phase_index].index].z + (uintptr_t)ins->phase[phase_index].state.sub[256], sizeof(struct state_s));
		if(ins->phase[phase_index].state.status==DEADEND){ goto pass_to_to;}
	}
	++phase_index;
	goto phase_inter;

	//to
	phase_to:
	while(ins->phase[ins->phasen-1].data->next){
		ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data->next;
		for(i=0;i<ins->phase[ins->phasen-1].data->len;i++){
			memcpy(&ins->phase[ins->phasen].state, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].z + (uintptr_t)ins->phase[ins->phasen].state.sub[*(ins->phase[ins->phasen-1].data->data+i)], sizeof(struct state_s));
			switch(ins->phase[ins->phasen].state.status){
				case DEADEND:
					goto pass_to_out;
					break;
				case CALLBACK:
					goto to_callback;
					break;
			}
		}
		to_x:
		switch(ins->phase[ins->phasen].state.status){
			case DEADEND:
				pass_to_out:
				ins->phase[ins->phasen].pend=0;
				if(ins->phase[ins->phasen].match){
					listcpy(ins->phasen, ins->phase[ins->phasen].match, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].data_z);
					ins->phase[ins->phasen].match=0;
					listfree(ins->phasen-1,ins->phase[ins->phasen].bak);
					ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;

					RESET(ins->phasen);

					goto phase_out;
				}else if(ins->phase[ins->phasen].index < ins->phase[ins->phasen].codecn){
					ins->phase[ins->phasen].index++;
					memcpy(&ins->phase[ins->phasen].state, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].z, sizeof(struct state_s));
					ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;
					continue;
				}else{
					ins->oerr++;

					RESET(ins->phasen);

					listfree(ins->phasen-1,ins->phase[ins->phasen-1].data->next);
					ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;

					continue;
				}
				break;
			case MATCH:
				ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
				listcpy(ins->phasen, ins->phase[ins->phasen].state.data, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].data_z);
				listfree(ins->phasen-1, ins->phase[ins->phasen].bak);
				ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;
				ins->phase[ins->phasen].pend=0;
				ins->phase[ins->phasen].match=NULL;
				RESET(ins->phasen);
				goto phase_out;
			case SUBMATCH:
				ins->phase[ins->phasen].match=ins->phase[ins->phasen].state.data;
				ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
				ins->phase[ins->phasen].pend=1;
				break;
			case CALLBACK:
				to_callback:
				ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].callback(ins);
				goto to_x;
			case NEXTPHASE:
				listfree(ins->phasen-1,ins->phase[ins->phasen-1].data->next);
				ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;
				RESET(ins->phasen);
				ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
				ins->phase[ins->phasen].pend=0;
				goto phase_out;
				break;
			case CONTINUE:
				ins->phase[ins->phasen].pend=1;
				break;
		}
		memcpy(&ins->phase[ins->phasen].state, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].z + (uintptr_t)ins->phase[ins->phasen].state.sub[256], sizeof(struct state_s));
		if(ins->phase[ins->phasen].state.status==DEADEND){ goto pass_to_out;}
	}

	phase_out:
	switch(ins->mode){
		case BSDCONV_BB:
			bb_out:
			while(ins->phase[ins->phasen].data_head->next){
				i=ins->back_len + ins->phase[ins->phasen].data_head->next->len;
				if(i > ins->out_len){
					goto bb_hibernate;
				}else{
					memcpy(ins->back + ins->back_len, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
					ins->back_len=i;
				}
				if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
					ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
				}
				data_ptr=ins->phase[ins->phasen].data_head->next;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}
			check_leftovers();
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				check_pending();
				return 0;
			}else{
			bb_hibernate:
				ins->from_data-=(ins->from_bak - ins->in_buf);
				i=(ins->feed+ins->feed_len)-ins->from_bak;
				memmove(ins->in_buf, ins->from_bak, i);
				ins->feed=ins->in_buf+i;
				ins->feed_len=ins->in_len - i;
				ins->from_bak=ins->in_buf;
				return 1;
			}
			break;
		case BSDCONV_BC:
			check_leftovers();
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				check_pending();
				i=0;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ptr=ins->out_buf=ins->back=malloc(i);
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					free(data_ptr);
				}
				return 0;
			}else{
			//bc_hibernate:
				ins->from_data-=(ins->from_bak - ins->in_buf);
				i=(ins->feed+ins->feed_len)-ins->from_bak;
				memmove(ins->in_buf, ins->from_bak, i);
				ins->feed=ins->in_buf+i;
				ins->feed_len=ins->in_len - i;
				ins->from_bak=ins->in_buf;
				return 1;
			}
			break;
		case BSDCONV_CB:
			cb_out:
			while(ins->phase[ins->phasen].data_head->next){
				i=ins->back_len + ins->phase[ins->phasen].data_head->next->len;
				if(i > ins->out_len){
					return 1;
				}else{
					memcpy(ins->back + ins->back_len, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
					ins->back_len=i;
				}
				if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
					ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
				}
				data_ptr=ins->phase[ins->phasen].data_head->next;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}

			
			check_leftovers();

			check_pending();
			return 0;
			break;
		case BSDCONV_CC:
			check_leftovers();

			check_pending();

			for(phase_index=0;phase_index<=ins->phasen;++phase_index){
				if(ins->phase[phase_index].pend){
					if(phase_index==0){
						goto pass_to_inter;
					}else if(phase_index==ins->phasen){
						goto pass_to_to;
					}else{
						goto pass_to_out;
					}
				}
			}
			i=0;
			data_ptr=ins->phase[ins->phasen].data_head->next;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->back_len=ins->out_len=i;
			ptr=ins->back=ins->out_buf=ins->back=malloc(i);
			data_ptr=ins->phase[ins->phasen].data_head;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				memcpy(ptr, data_ptr->data, data_ptr->len);
				ptr+=data_ptr->len;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				free(data_ptr);
			}
			return 0;
			break;
		case BSDCONV_BM:
			check_leftovers();
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				check_pending();
				i=0;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ins->back_len=i;
				return 0;
			}else{
				ins->from_data-=(ins->from_bak - ins->in_buf);
				i=(ins->feed+ins->feed_len)-ins->from_bak;
				memmove(ins->in_buf, ins->from_bak, i);
				ins->feed=ins->in_buf+i;
				ins->feed_len=ins->in_len - i;
				ins->from_bak=ins->in_buf;
				return 1;
			}
			break;
		case BSDCONV_CM:
			check_leftovers();
			check_pending();
			i=0;
			data_ptr=ins->phase[ins->phasen].data_head->next;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->back_len=i;
			return 0;
			break;
	}
	return 1;
}

char * bsdconv_error(void){
	switch(errno){
		case EOPNOTSUPP:
				return strdup("Unsupported charset/encoding");
		case ENOMEM:
				return strdup("Mmap failed");
		case EINVAL:
				return strdup("Conversion syntax error");
		default:
				return strdup("Unknown error");;
	}
}
