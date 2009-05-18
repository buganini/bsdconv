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
	ret->n##X=n##X;		\
}while(0);

#define GENLIST(X) do{	\
	X=strdup(o##X);	\
	ret->X=malloc(n##X * sizeof(struct bsdconv_codec_t));		\
	ret->X[0].desc=X;	\
	chdir(#X);	\
	brk=0;	\
	for(i=0,t=X;;++t){	\
		if(*t==',' || *t==0){	\
			if(*t==0){	\
				brk=1;	\
			}	\
			*t=0;	\
			strcpy(buf, ret->X[i].desc);	\
			realpath(buf, path);	\
			ret->X[i].fd=open(path, O_RDONLY);	\
			if(!ret->X[i].fd){	\
				fprintf(stderr, "No such codec %s/%s", #X, ret->X[i].desc);	\
				exit(1);	\
			}	\
			fstat(ret->X[i].fd, &stat);		\
			ret->X[i].z=mmap(0,stat.st_size,PROT_READ, MAP_PRIVATE,ret->X[i].fd,0);	\
			if(!ret->X[i].z){	\
				fprintf(stderr, "Memory map failed for %s/%s", #X, ret->X[i].desc);	\
				exit(1);	\
			}	\
			strcat(path, ".so");	\
			ret->X[i].callback=NULL;	\
			ret->X[i].cbinit=NULL;	\
			ret->X[i].cbclear=NULL;	\
			if((ret->X[i].dl=dlopen(path	, RTLD_LAZY))){	\
				ret->X[i].callback=dlsym(ret->X[i].dl,"callback");	\
				ret->X[i].cbinit=dlsym(ret->X[i].dl,"cbinit");	\
				ret->X[i].cbclear=dlsym(ret->X[i].dl,"cbclear");	\
			}	\
			if(i+1 < n##X){	\
				ret->X[++i].desc=t+1;	\
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

#define listcpy(X,Y,Z) for(data_ptr=(Y);data_ptr;){	\
	ins->X##_data_tail->next=malloc(sizeof(struct data_s));	\
	ins->X##_data_tail=ins->X##_data_tail->next;	\
	memcpy(ins->X##_data_tail, (unsigned char *)((Z)+(unsigned int)data_ptr), sizeof(struct data_s));	\
	data_ptr=ins->X##_data_tail->next;	\
	ins->X##_data_tail->next=NULL;	\
	ptr=(unsigned char *)((Z)+(unsigned int)ins->X##_data_tail->data);	\
	ins->X##_data_tail->data=malloc(ins->X##_data_tail->len);	\
	memcpy(ins->X##_data_tail->data, ptr, ins->X##_data_tail->len);	\
}

#define listfree(X,Y)	while(ins->X##_data_head->next!=(struct data_s *)(Y)){	\
	data_ptr=ins->X##_data_head->next->next;	\
	free(ins->X##_data_head->next->data);	\
	if(ins->X##_data_tail==ins->X##_data_head->next){	\
		ins->X##_data_tail=ins->X##_data_head;	\
	}	\
	free(ins->X##_data_head->next);	\
	ins->X##_data_head->next=data_ptr;	\
}

#define RESET(X) do{	\
	ins->X##_index=0;	\
	memcpy(&ins->X##_state, cd->X[ins->X##_index].z, sizeof(struct state_s));	\
}while(0);

void bsdconv_init(struct bsdconv_t *cd, struct bsdconv_instruction *ins, unsigned char *inbuf, size_t inlen, unsigned char *outbuf, size_t outlen){
	int i;
	
	ins->in_buf=inbuf;
	ins->in_len=inlen;
	ins->out_buf=outbuf;	// *
	ins->out_len=outlen;
	ins->feed=inbuf;
	ins->feed_len=inlen;
	ins->back=outbuf;	// *
	ins->back_len=0;
	/* (*)never changed */
	
	ins->fpriv=malloc((cd->nfrom+1) * sizeof(void *));
	ins->ipriv=malloc((cd->ninter+1) * sizeof(void *));
	ins->tpriv=malloc((cd->nto+1) * sizeof(void *));
	for(i=0;i<=cd->nfrom;i++){
		if(cd->from[i].cbinit){
			ins->fpriv[i]=cd->from[i].cbinit();
		}
	}
	for(i=0;i<=cd->ninter;i++){
		if(cd->inter[i].cbinit)
			ins->ipriv[i]=cd->inter[i].cbinit();
	}
	for(i=0;i<=cd->nto;i++){
		if(cd->to[i].cbinit)
			ins->tpriv[i]=cd->to[i].cbinit();
	}

	ins->ierr=0;
	ins->oerr=0;

	ins->inter_data_head=&ins->inter_data_ent;
	ins->to_data_head=&ins->to_data_ent;
	ins->out_data_head=&ins->out_data_ent;

	ins->inter_data_head->next=
	ins->to_data_head->next=
	ins->out_data_head->next=
	NULL;

	ins->from_bak=ins->from_data=ins->in_buf;
	ins->inter_bak=ins->inter_data=ins->inter_data_tail=ins->inter_data_head;
	ins->to_bak=ins->to_data=ins->to_data_tail=ins->to_data_head;
	ins->out_data_tail=ins->out_data_head;

	ins->from_match=NULL;
	ins->inter_match=NULL;
	ins->to_match=NULL;

	RESET(from);
	RESET(inter);
	RESET(to);
}

struct bsdconv_t *bsdconv_create(const char *conversion){
	struct bsdconv_t *ret=malloc(sizeof(struct bsdconv_t));
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
	
	ret->nfrom--;
	ret->nto--;
	ret->ninter--;
	return ret;
}

void bsdconv_destroy(struct bsdconv_t *cd){
	free(cd->from[0].desc);
	free(cd);
}

int bsd_conv(struct bsdconv_t *cd, struct bsdconv_instruction *ins){
	unsigned int i;
	struct data_s *data_ptr;
	unsigned char *ptr;

	ins->back_len=0;

	if(ins->out_data_head->next) goto phase_out;
	if(ins->to_data_head->next) goto phase_to;
	if(ins->inter_data_head->next) goto phase_inter;
	if(ins->in_buf < ins->feed+ins->feed_len) goto phase_from;

	if(ins->pend_from) goto pass_to_inter;
	if(ins->pend_inter) goto pass_to_to;
	if(ins->pend_to) goto pass_to_out;

	return 0;

#define FROM_NEXT() do{	\
++ins->from_data;	\
}while(0);	\
	//from
	phase_from:
	while(ins->from_data < ins->feed+ins->feed_len){
		memcpy(&ins->from_state, cd->from[ins->from_index].z + (unsigned int)ins->from_state.sub[*ins->from_data], sizeof(struct state_s));
		from_x:
		switch(ins->from_state.status){
			case DEADEND:
				pass_to_inter:
				ins->pend_from=0;
				if(ins->from_match){
					listcpy(inter, ins->from_match, cd->from[ins->from_index].z);
					ins->from_match=NULL;
					RESET(from);

					ins->from_data=ins->from_bak;
					goto phase_inter;
				}else if(ins->from_index < cd->nfrom){
					ins->from_index++;
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));

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
				listcpy(inter, ins->from_state.data, cd->from[ins->from_index].z);
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
				cd->from[ins->from_index].callback(ins);
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
			memcpy(&ins->inter_state, cd->inter[ins->inter_index].z + (unsigned int)ins->inter_state.sub[*(ins->inter_data->data+i)], sizeof(struct state_s));
			if(ins->inter_state.status==DEADEND){
				break;
			}
		}
		switch(ins->inter_state.status){
			case DEADEND:
				pass_to_to:
				ins->pend_inter=0;
				if(ins->inter_match){
					listcpy(to, ins->inter_match, cd->inter[ins->inter_index].z);
					ins->inter_match=NULL;
					listfree(inter,ins->inter_bak);
					ins->inter_data=ins->inter_bak;

					RESET(inter);

					ins->inter_data=ins->inter_data_head;
					goto phase_to;
				}else if(ins->inter_index < cd->ninter){
					ins->inter_index++;
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));
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
				listcpy(to, ins->inter_state.data, cd->inter[ins->inter_index].z);
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
		memcpy(&ins->inter_state, cd->inter[ins->inter_index].z + (unsigned int)ins->inter_state.sub[256], sizeof(struct state_s));
		if(ins->inter_state.status==DEADEND){ goto pass_to_to;}
	}

	//to
	phase_to:
	while(ins->to_data->next){
		ins->to_data=ins->to_data->next;
		for(i=0;i<ins->to_data->len;i++){
			memcpy(&ins->to_state, cd->to[ins->to_index].z + (unsigned int)ins->to_state.sub[*(ins->to_data->data+i)], sizeof(struct state_s));
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
					listcpy(out, ins->to_match, cd->to[ins->to_index].z);
					ins->to_match=0;
					listfree(to,ins->to_bak);
					ins->to_data=ins->to_data_head;

					RESET(to);

					goto phase_out;
				}else if(ins->to_index < cd->nto){
					ins->to_index++;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));
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
				listcpy(out, ins->to_state.data, cd->to[ins->to_index].z);
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
				cd->to[ins->to_index].callback(ins);
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
		memcpy(&ins->to_state, cd->to[ins->to_index].z + (unsigned int)ins->to_state.sub[256], sizeof(struct state_s));
		if(ins->to_state.status==DEADEND){ goto pass_to_out;}
	}

	//out
	phase_out:
	while(ins->out_data_head->next){
		i=ins->back_len + ins->out_data_head->next->len;
		if(i > ins->out_len){
			goto hibernate;
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

	hibernate:
		ins->from_data-=(ins->from_bak - ins->in_buf);
		i=(ins->feed+ins->feed_len)-ins->from_bak;
		memmove(ins->in_buf, ins->from_bak, i);
		ins->feed=ins->in_buf+i;
		ins->feed_len=ins->in_len - i;
		ins->from_bak=ins->in_buf;
		return 1;
}
