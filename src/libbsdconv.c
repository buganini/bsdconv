#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include "bsdconv.h"

#define COUNT(X) do{	\
	if(*o##X){	\
		n##X=1;	\
		for(t=(char *)o##X;*t;t++){	\
			if(*t==','){	\
				n##X++;	\
			}	\
		}	\
	}else{	\
		n##X=0;	\
	}	\
	ins->n##X=n##X;		\
}while(0);

#define GENLIST(X) do{	\
	X=strdup(o##X);	\
	ins->X=malloc(n##X * sizeof(struct bsdconv_codec_t));		\
	ins->X[0].desc=X;	\
	chdir(#X);	\
	brk=0;	\
	for(i=0,t=X;;++t){	\
		if(*t==',' || *t==0){	\
			if(*t==0){	\
				brk=1;	\
			}	\
			*t=0;	\
			strcpy(buf, ins->X[i].desc);	\
			realpath(buf, path);	\
			ins->X[i].fd=open(path, O_RDONLY);	\
			if(!ins->X[i].fd){	\
				fprintf(stderr, "No such codec %s/%s", #X, ins->X[i].desc);	\
				exit(1);	\
			}	\
			fstat(ins->X[i].fd, &stat);		\
			ins->X[i].z=mmap(0,stat.st_size,PROT_READ, MAP_PRIVATE,ins->X[i].fd,0);	\
			if(!ins->X[i].z){	\
				fprintf(stderr, "Memory map failed for %s/%s", #X, ins->X[i].desc);	\
				exit(1);	\
			}	\
			strcat(path, ".so");	\
			ins->X[i].cbcreate=NULL;	\
			ins->X[i].cbinit=NULL;	\
			ins->X[i].callback=NULL;	\
			ins->X[i].cbdestroy=NULL;	\
			if((ins->X[i].dl=dlopen(path	, RTLD_LAZY))){	\
				ins->X[i].callback=dlsym(ins->X[i].dl,"callback");	\
				ins->X[i].cbcreate=dlsym(ins->X[i].dl,"cbcreate");	\
				ins->X[i].cbinit=dlsym(ins->X[i].dl,"cbinit");	\
				ins->X[i].cbdestroy=dlsym(ins->X[i].dl,"cbdestroy");	\
			}	\
			if(i+1 < n##X){	\
				ins->X[++i].desc=t+1;	\
			}	\
			if(brk){	\
				break;	\
			}	\
		}else{	\
			*t=toupper(*t); 	\
		}	\
	}	\
	chdir("..");	\
}while(0);

#define RESET(X) do{	\
	ins->X##_index=0;	\
	memcpy(&ins->X##_state, ins->X[ins->X##_index].z, sizeof(struct state_s));	\
}while(0);

void bsdconv_init(struct bsdconv_instance *ins){
	int i;
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
		case BSDCONV_CC:
			break;
	}

	ins->ierr=0;
	ins->oerr=0;

	ins->inter_data_head->next=
	ins->to_data_head->next=
	ins->out_data_head->next=
	NULL;

	ins->from_bak=ins->from_data=ins->feed;
	ins->inter_bak=ins->inter_data=ins->inter_data_tail=ins->inter_data_head;
	ins->to_bak=ins->to_data=ins->to_data_tail=ins->to_data_head;
	ins->out_data_tail=ins->out_data_head;

	ins->from_match=NULL;
	ins->inter_match=NULL;
	ins->to_match=NULL;

	RESET(from);
	RESET(inter);
	RESET(to);

	for(i=0;i<=ins->nfrom;i++){
		if(ins->from[i].cbinit){
			ins->from[i].cbinit(ins->fpriv[i]);
		}
	}
	for(i=0;i<=ins->ninter;i++){
		if(ins->inter[i].cbinit)
			ins->inter[i].cbinit(ins->ipriv[i]);
	}
	for(i=0;i<=ins->nto;i++){
		if(ins->to[i].cbinit)
			ins->to[i].cbinit(ins->tpriv[i]);
	}
}

struct bsdconv_instance *bsdconv_create(const char *conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	struct stat stat;
	char *ofrom, *ointer, *oto;
	char *t, *from, *inter, *to;
	int i,nfrom,nto,ninter, brk;
	char buf[64], path[512];

	t=strdup(conversion);
	ofrom=(char *)strsep(&t, ":");
	ointer=(char *)strsep(&t, ":");
	oto=(char *)strsep(&t, ":");

	COUNT(from);
	COUNT(inter);
	COUNT(to);

	if(ninter==0){
		ninter=2;
		ointer="DUMMY";
	}

	chdir(PREFIX "/share/bsdconv");
	if(nfrom==0 || nto==0){
		fprintf(stderr, "Need at least 1 from and to encoding.\n");
		fflush(stderr);
		return NULL;
	}
	GENLIST(from);
	GENLIST(inter);
	GENLIST(to);
	
	ins->nfrom--;
	ins->nto--;
	ins->ninter--;

	ins->inter_data_head=malloc(sizeof(struct data_s));
	ins->to_data_head=malloc(sizeof(struct data_s));
	ins->out_data_head=malloc(sizeof(struct data_s));

	ins->fpriv=malloc((ins->nfrom+1) * sizeof(void *));
	ins->ipriv=malloc((ins->ninter+1) * sizeof(void *));
	ins->tpriv=malloc((ins->nto+1) * sizeof(void *));

	for(i=0;i<=ins->nfrom;i++){
		if(ins->from[i].cbcreate){
			ins->fpriv[i]=ins->from[i].cbcreate();
		}
	}
	for(i=0;i<=ins->ninter;i++){
		if(ins->inter[i].cbcreate)
			ins->ipriv[i]=ins->inter[i].cbcreate();
	}
	for(i=0;i<=ins->nto;i++){
		if(ins->to[i].cbcreate)
			ins->tpriv[i]=ins->to[i].cbcreate();
	}

	return ins;
}

void bsdconv_destroy(struct bsdconv_instance *ins){
	free(ins->from[0].desc);
	free(ins);
}

int bsd_conv(struct bsdconv_instance *ins){
	unsigned int i;
	struct data_s *data_ptr;
	unsigned char *ptr;

	switch(ins->mode){
		case BSDCONV_BB:
			ins->back_len=0;
			if(ins->out_data_head->next) goto bb_out;
			if(ins->to_data_head->next) goto phase_to;
			if(ins->inter_data_head->next) goto phase_inter;
			break;
		case BSDCONV_BC:
			if(ins->to_data_head->next) goto phase_to;
			if(ins->inter_data_head->next) goto phase_inter;
			break;
		case BSDCONV_CB:
			ins->back_len=0;
			if(ins->out_data_head->next) goto cb_out;
			break;
		case BSDCONV_CC:
			break;
		}

#define FROM_NEXT() do{	\
++ins->from_data;	\
}while(0);	\
	//from
	phase_from:
	while(ins->from_data < ins->feed+ins->feed_len){
		memcpy(&ins->from_state, ins->from[ins->from_index].z + (unsigned int)ins->from_state.sub[*ins->from_data], sizeof(struct state_s));
		from_x:
		switch(ins->from_state.status){
			case DEADEND:
				pass_to_inter:
				ins->pend_from=0;
				if(ins->from_match){
					listcpy(inter, ins->from_match, ins->from[ins->from_index].z);
					ins->from_match=NULL;
					RESET(from);

					ins->from_data=ins->from_bak;
					goto phase_inter;
				}else if(ins->from_index < ins->nfrom){
					ins->from_index++;
					memcpy(&ins->from_state, ins->from[ins->from_index].z, sizeof(struct state_s));

					ins->from_data=ins->from_bak;
					continue;
				}else{
					ins->ierr++;

					RESET(from);

					ins->from_data=ins->from_bak;
					FROM_NEXT();
					ins->from_bak=ins->from_data;
					continue;
				}
				break;
			case MATCH:
				FROM_NEXT();
				ins->from_bak=ins->from_data;
				listcpy(inter, ins->from_state.data, ins->from[ins->from_index].z);
				ins->pend_from=0;
				ins->from_match=NULL;
				RESET(from);
				goto phase_inter;
			case SUBMATCH:
				ins->from_match=ins->from_state.data;
				FROM_NEXT();
				ins->from_bak=ins->from_data;
				ins->pend_from=1;
				break;
			case CALLBACK:
				ins->from[ins->from_index].callback(ins);
				goto from_x;
			case NEXTPHASE:
				RESET(from);
				FROM_NEXT();
				ins->from_bak=ins->from_data;
				ins->pend_from=0;
				goto phase_inter;
				break;
			case CONTINUE:
				ins->pend_from=1;
				FROM_NEXT();
				break;
			default:
				FROM_NEXT();
		}
	}

	//inter
	phase_inter:
	while(ins->inter_data->next){
		ins->inter_data=ins->inter_data->next;
		for(i=0;i<ins->inter_data->len;i++){
			memcpy(&ins->inter_state, ins->inter[ins->inter_index].z + (unsigned int)ins->inter_state.sub[*(ins->inter_data->data+i)], sizeof(struct state_s));
			if(ins->inter_state.status==DEADEND){
				break;
			}
		}
		switch(ins->inter_state.status){
			case DEADEND:
				pass_to_to:
				ins->pend_inter=0;
				if(ins->inter_match){
					listcpy(to, ins->inter_match, ins->inter[ins->inter_index].z);
					ins->inter_match=NULL;
					listfree(inter,ins->inter_bak);
					ins->inter_data=ins->inter_bak;

					RESET(inter);

					ins->inter_data=ins->inter_data_head;
					goto phase_to;
				}else if(ins->inter_index < ins->ninter){
					ins->inter_index++;
					memcpy(&ins->inter_state, ins->inter[ins->inter_index].z, sizeof(struct state_s));
					ins->inter_data=ins->inter_bak;
					ins->inter_data=ins->inter_data_head;
					continue;
				}else{
					data_ptr=ins->inter_data_head->next;
					ins->inter_data_head->next=ins->inter_data_head->next->next;
					ins->inter_data=ins->inter_data_head;
					data_ptr->next=NULL;
					ins->to_data_tail->next=data_ptr;
					if(ins->inter_data_tail==data_ptr){
						ins->inter_data_tail=ins->inter_data_head;
					}
					ins->inter_bak=ins->inter_data=ins->inter_data_head;

					RESET(inter);

					goto phase_to;
				}
				break;
			case MATCH:
				ins->inter_data=ins->inter_bak=ins->inter_data->next;
				listcpy(to, ins->inter_state.data, ins->inter[ins->inter_index].z);
				listfree(inter,ins->inter_bak);
				ins->pend_inter=0;
				ins->inter_match=NULL;

				RESET(inter);

				ins->inter_data=ins->inter_data_head;
				goto phase_to;
			case SUBMATCH:
				ins->inter_match=ins->inter_state.data;
				ins->inter_bak=ins->inter_data->next;
				ins->pend_inter=1;
				break;
			case NEXTPHASE:
				goto phase_to;
				break;
			case CONTINUE:
				ins->pend_inter=1;
				break;
		}
		memcpy(&ins->inter_state, ins->inter[ins->inter_index].z + (unsigned int)ins->inter_state.sub[256], sizeof(struct state_s));
		if(ins->inter_state.status==DEADEND){ goto pass_to_to;}
	}

	//to
	phase_to:
	while(ins->to_data->next){
		ins->to_data=ins->to_data->next;
		for(i=0;i<ins->to_data->len;i++){
			memcpy(&ins->to_state, ins->to[ins->to_index].z + (unsigned int)ins->to_state.sub[*(ins->to_data->data+i)], sizeof(struct state_s));
			switch(ins->to_state.status){
				case DEADEND:
					goto pass_to_out;
					break;
				case CALLBACK:
					goto to_callback;
					break;
			}
		}
		to_x:
		switch(ins->to_state.status){
			case DEADEND:
				pass_to_out:
				ins->pend_to=0;
				if(ins->to_match){
					listcpy(out, ins->to_match, ins->to[ins->to_index].z);
					ins->to_match=0;
					listfree(to,ins->to_bak);
					ins->to_data=ins->to_data_head;

					RESET(to);

					goto phase_out;
				}else if(ins->to_index < ins->nto){
					ins->to_index++;
					memcpy(&ins->to_state, ins->to[ins->to_index].z, sizeof(struct state_s));
					ins->to_data=ins->to_data_head;
					continue;
				}else{
					ins->oerr++;

					RESET(to);

					listfree(to,ins->to_data->next);
					ins->to_bak=ins->to_data=ins->to_data_head;

					continue;
				}
				break;
			case MATCH:
				ins->to_bak=ins->to_data->next;
				listcpy(out, ins->to_state.data, ins->to[ins->to_index].z);
				listfree(to, ins->to_bak);
				ins->to_data=ins->to_data_head;
				ins->pend_to=0;
				ins->to_match=NULL;
				RESET(to);
				goto phase_out;
			case SUBMATCH:
				ins->to_match=ins->to_state.data;
				ins->to_bak=ins->to_data->next;
				ins->pend_to=1;
				break;
			case CALLBACK:
				to_callback:
				ins->to[ins->to_index].callback(ins);
				goto to_x;
			case NEXTPHASE:
				listfree(to,ins->to_data->next);
				ins->to_data=ins->to_data_head;
				RESET(to);
				ins->to_bak=ins->to_data->next;
				ins->pend_to=0;
				goto phase_out;
				break;
			case CONTINUE:
				ins->pend_to=1;
				break;
		}
		memcpy(&ins->to_state, ins->to[ins->to_index].z + (unsigned int)ins->to_state.sub[256], sizeof(struct state_s));
		if(ins->to_state.status==DEADEND){ goto pass_to_out;}
	}

	phase_out:

	switch(ins->mode){
		case BSDCONV_BB:
			bb_out:
			while(ins->out_data_head->next){
				i=ins->back_len + ins->out_data_head->next->len;
				if(i > ins->out_len){
					goto bb_hibernate;
				}else{
					memcpy(ins->back + ins->back_len, ins->out_data_head->next->data, ins->out_data_head->next->len);
					ins->back_len=i;
				}
				if(ins->out_data_tail==ins->out_data_head->next){
					ins->out_data_tail=ins->out_data_head;
				}
				data_ptr=ins->out_data_head->next;
				ins->out_data_head->next=ins->out_data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}
			if(ins->to_data->next) goto phase_to;
			if(ins->inter_data->next) goto phase_inter;
			if(ins->from_data < ins->feed+ins->feed_len) goto phase_from;
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				if(ins->pend_from) goto pass_to_inter;
				if(ins->pend_inter) goto pass_to_to;
				if(ins->pend_to) goto pass_to_out;
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
			if(ins->to_data->next) goto phase_to;
			if(ins->inter_data->next) goto phase_inter;
			if(ins->from_data < ins->feed+ins->feed_len) goto phase_from;
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				if(ins->pend_from) goto pass_to_inter;
				if(ins->pend_inter) goto pass_to_to;
				if(ins->pend_to) goto pass_to_out;
				i=0;
				data_ptr=ins->out_data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ptr=ins->out_buf=ins->back=malloc(i);
				data_ptr=ins->out_data_head;
				while(ins->out_data_head->next){
					data_ptr=ins->out_data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->out_data_head->next=ins->out_data_head->next->next;
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
			while(ins->out_data_head->next){
				i=ins->back_len + ins->out_data_head->next->len;
				if(i > ins->out_len){
					return 1;
				}else{
					memcpy(ins->back + ins->back_len, ins->out_data_head->next->data, ins->out_data_head->next->len);
					ins->back_len=i;
				}
				if(ins->out_data_tail==ins->out_data_head->next){
					ins->out_data_tail=ins->out_data_head;
				}
				data_ptr=ins->out_data_head->next;
				ins->out_data_head->next=ins->out_data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}
			if(ins->to_data->next) goto phase_to;
			if(ins->inter_data->next) goto phase_inter;
			if(ins->from_data < ins->feed+ins->feed_len) goto phase_from;
			if(ins->pend_from) goto pass_to_inter;
			if(ins->pend_inter) goto pass_to_to;
			if(ins->pend_to) goto pass_to_out;
			return 0;
			break;
		case BSDCONV_CC:
			if(ins->to_data->next) goto phase_to;
			if(ins->inter_data->next) goto phase_inter;
			if(ins->from_data < ins->feed+ins->feed_len) goto phase_from;
			if(ins->pend_from) goto pass_to_inter;
			if(ins->pend_inter) goto pass_to_to;
			if(ins->pend_to) goto pass_to_out;
			i=0;
			data_ptr=ins->out_data_head;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->back_len=ins->out_len=i;
			ptr=ins->back=ins->out_buf=ins->back=malloc(i);
			data_ptr=ins->out_data_head;
			while(ins->out_data_head->next){
				data_ptr=ins->out_data_head->next;
				memcpy(ptr, data_ptr->data, data_ptr->len);
				ptr+=data_ptr->len;
				ins->out_data_head->next=ins->out_data_head->next->next;
				free(data_ptr);
			}
			return 0;
			break;
	}
	return 1;
}
