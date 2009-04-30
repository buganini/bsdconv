#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
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
			ret->X[i].fd=open(ret->X[i].desc, O_RDONLY);	\
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
	*(ins->X##_data_tail)=malloc(sizeof(struct data_s));	\
printf("A:%p %p\n",data_ptr, *(ins->X##_data_tail));	\
printf("B:%p\n",*(ins->X##_data_tail));	\
	memcpy( *(ins->X##_data_tail), (unsigned char *)((Z)+(unsigned int)data_ptr), sizeof(struct data_s));	\
printf("B:%p\n",*(ins->X##_data_tail));	\
	ptr=(unsigned char *)((Z)+(unsigned int)(*(ins->X##_data_tail))->data);	\
printf("C:\n");	\
	(*(ins->X##_data_tail))->data=malloc( (*(ins->X##_data_tail))->len );	\
printf("D:%p\n",ptr);	\
	memcpy( (*(ins->X##_data_tail))->data, ptr, (*(ins->X##_data_tail))->len );	\
	\
	data_ptr=(*(ins->X##_data_tail))->next;	\
	ins->X##_data_tail=&( (*(ins->X##_data_tail))->next );	\
	*(ins->X##_data_tail)=NULL;	\
}

#define listfree(X,Y) while(ins->X##_data_head && data_ptr!=(struct data_s *)(Y)){	\
	data_ptr=ins->X##_data_head->next;	\
	free(ins->X##_data_head->data);	\
	free(ins->X##_data_head);	\
	ins->X##_data_head=data_ptr;	\
}

void bsdconv_init(struct bsdconv_t *cd, struct bsdconv_instruction *ins, unsigned char *inbuf, size_t inlen, unsigned char *outbuf, size_t outlen){
	ins->in_buf=inbuf;
	ins->in_len=inlen;
	ins->out_buf=outbuf;	// *
	ins->out_len=outlen;
	ins->feed=inbuf;
	ins->feed_len=inlen;
	ins->back=outbuf;	// *
	ins->back_len=0;
	/* (*)never changed */

	ins->ierr=0;
	ins->oerr=0;

	ins->from_index=0;
	ins->inter_index=0;
	ins->to_index=0;

	ins->inter_data_head=NULL;
	ins->to_data_head=NULL;
	ins->out_data_head=NULL;
	ins->inter_data_tail=&ins->inter_data_head;
	ins->to_data_tail=&ins->to_data_head;
	ins->out_data_tail=&ins->out_data_head;

	ins->from_match.data=0;
	ins->inter_match.data=0;
	ins->to_match.data=0;

	ins->from_match.sub[0]=(struct state_s *)ins->feed;
	ins->from_match.sub[1]=(struct state_s *)ins->feed_len;

	memcpy(&ins->from_state, cd->from[0].z, sizeof(struct state_s));
	memcpy(&ins->inter_state, cd->inter[0].z, sizeof(struct state_s));
	memcpy(&ins->to_state, cd->to[0].z, sizeof(struct state_s));
}

struct bsdconv_t *bsdconv_create(const char *conversion){
	struct bsdconv_t *ret=malloc(sizeof(struct bsdconv_t));
	struct stat stat;
	char *ofrom, *ointer, *oto;
	char *t, *from, *inter, *to;
	int i,nfrom,nto,ninter, brk;

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

	chdir("/usr/local/share/bsdconv");
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
	struct data_s *inter_data, *to_data, *out_data;

	struct data_s iterminator={
		.data=(unsigned char *)"\x01\x3f",
		.len=2,
		.next=0,
	};
	struct data_s oterminator={
		.data=(unsigned char *)"?",
		.len=1,
		.next=0,
	};

	ins->back_len=0;

	ins->feed_len+=ins->feed - ins->in_buf;
	ins->feed=ins->in_buf;
	ins->from_match.sub[1]=(struct state_s *)ins->feed_len;

	if(ins->out_data_head){
		goto phase_out;
	}
	if(ins->pend_from){
		goto pass_to_inter;
	}
	if(ins->pend_inter){
		goto pass_to_to;
	}
	if(ins->pend_to){
		goto pass_to_out;
	}

#define FROM_NEXT() do{	\
	--ins->feed_len;	\
	++ins->feed;	\
}while(0);	\
	//from
	phase_from:
printf("%d\n", __LINE__);
	while(ins->feed_len){
		memcpy(&ins->from_state, cd->from[ins->from_index].z + (unsigned int)ins->from_state.sub[*ins->feed], sizeof(struct state_s));
		switch(ins->from_state.status){
			case DEADEND:
				pass_to_inter:
				ins->pend_from=0;
				if(ins->from_match.data){
printf("%p\n",cd->from[ins->from_index].z);
					listcpy(inter, ins->from_match.data, cd->from[ins->from_index].z);
					ins->from_match.data=NULL;
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));
					ins->feed=(unsigned char *)ins->from_match.sub[0];
					ins->feed_len=(unsigned int)ins->from_match.sub[1];
					ins->from_index=0;
					goto phase_inter;
				}else if(ins->from_index < cd->nfrom){
					ins->from_index++;
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));
					ins->feed=(unsigned char *)ins->from_match.sub[0];
					ins->feed_len=(unsigned int)ins->from_match.sub[1];
					continue;
				}else{
					ins->from_index=0;
					ins->ierr++;

					listcpy(inter, &iterminator, 0);
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));

					ins->feed=(unsigned char *)ins->from_match.sub[0];
					ins->feed_len=(unsigned int)ins->from_match.sub[1];
					FROM_NEXT();
					ins->from_match.sub[0]=(struct state_s *)ins->feed;
					ins->from_match.sub[1]=(struct state_s *)ins->feed_len;
					goto phase_inter;
				}
				break;
			case MATCH:
			case SUBMATCH:
				ins->from_match.data=ins->from_state.data;
				FROM_NEXT();
				ins->from_match.sub[0]=(struct state_s *)ins->feed;
				ins->from_match.sub[1]=(struct state_s *)ins->feed_len;
				break;
			case CALLBACK:
			case CONTINUE:
				ins->pend_from=1;
				FROM_NEXT();
			default:
				FROM_NEXT();
		}
	}

#define INTER_NEXT() if((inter_data=inter_data->next)){	\
	memcpy(&ins->inter_state, (unsigned int)cd->inter[ins->inter_index].z + ins->inter_state.sub[256], sizeof(struct state_s));	\
	if(ins->inter_state.status==DEADEND){ goto pass_to_to;}	\
}
	//inter
	phase_inter:
printf("%d\n", __LINE__);
	ins->inter_match.sub[0]=(struct state_s *)ins->inter_data_head;
	inter_data=ins->inter_data_head;
	while(inter_data){
		for(i=0;i<inter_data->len;i++){
			memcpy(&ins->inter_state, cd->inter[ins->inter_index].z + (unsigned int)ins->inter_state.sub[*(inter_data->data+i)], sizeof(struct state_s));
			if(ins->inter_state.status==DEADEND){
				break;
			}
		}
		switch(ins->inter_state.status){
			case DEADEND:
				pass_to_to:
				ins->pend_inter=0;
				if(ins->inter_match.data){
					listcpy(to, ins->inter_match.data, cd->inter[ins->inter_index].z);
					ins->inter_match.data=0;
					listfree(inter,ins->inter_match.sub[0]);

					ins->inter_index=0;
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));

					goto phase_to;
				}else if(ins->inter_index < cd->ninter){
					ins->inter_index++;
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));
					inter_data=ins->inter_data_head;
					continue;
				}else{
					ins->to_data_head=ins->inter_data_head;
					ins->inter_data_head=ins->inter_data_head->next;
					ins->to_data_head->next=0;

					ins->inter_index=0;
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));

					goto phase_to;
				}
				break;
			case MATCH:
			case SUBMATCH:
				ins->inter_match.data=ins->inter_state.data;
				INTER_NEXT();
				ins->inter_match.sub[0]=(struct state_s *)inter_data;
				break;
			case CONTINUE:
				ins->pend_inter=1;
				INTER_NEXT();
				break;
			default:
				INTER_NEXT();
				break;
		}
	}

#define TO_NEXT() if((to_data=inter_data->next)){     \
	memcpy(&ins->to_state, (unsigned int)cd->to[ins->to_index].z + ins->to_state.sub[256], sizeof(struct state_s));   \
	if(ins->to_state.status==DEADEND){ goto pass_to_out;} \
}
	//to
	phase_to:
printf("%d\n", __LINE__);
	ins->to_match.sub[0]=(struct state_s *)ins->to_data_head;
	to_data=ins->to_data_head;
	while(to_data){
		for(i=0;i<to_data->len;i++){
			memcpy(&ins->to_state, cd->to[ins->to_index].z + (unsigned int)ins->to_state.sub[*(to_data->data+i)], sizeof(struct state_s));
			if(ins->to_state.status==DEADEND){
				break;
			}
		}
		switch(ins->to_state.status){
			case DEADEND:
				pass_to_out:
				ins->pend_to=0;
				if(ins->to_match.data){
					listcpy(out, ins->to_match.data, cd->to[ins->to_index].z);
					ins->to_match.data=0;
					listfree(to,ins->to_match.sub[0]);

					ins->to_index=0;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));

					goto phase_out;
				}else if(ins->to_index < cd->nto){
					ins->to_index++;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));
					to_data=ins->to_data_head;
					continue;
				}else{
					ins->oerr++;
					listcpy(out, &oterminator, 0);

					ins->to_index=0;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));

					listfree(to,to_data->next);
					to_data=ins->to_data_head;

					goto phase_out;
				}
				break;
			case MATCH:
			case SUBMATCH:
				ins->to_match.data=ins->to_state.data;
				TO_NEXT();
				ins->to_match.sub[0]=(struct state_s *)to_data;
				break;
			case CALLBACK:
			case CONTINUE:
				ins->pend_to=1;
				TO_NEXT();
				break;
			default:
				TO_NEXT();
				break;
		}
	}

	//out
	phase_out:
	out_data=ins->out_data_head;
	while(out_data){
		i=ins->back_len + out_data->len;
		if(i > ins->out_len){
			goto hibernate;
		}else{
			memcpy(ins->back + ins->back_len, out_data->data, out_data->len);
			ins->back_len=i;
		}
		data_ptr=out_data;
		out_data=out_data->next;
		free(data_ptr);
	}

	if(ins->to_data_head){
		goto phase_to;
	}
	if(ins->inter_data_head){
		goto phase_inter;
	}
	if(ins->feed_len){
		goto phase_from;
	}

	if(ins->pend_from || ins->pend_inter || ins->pend_to){
		goto hibernate;
	}

	return 0;

	hibernate:
		if(ins->pend_from){
			/* XXX use memmove */
			memcpy(ins->in_buf, (char *) ins->from_match.sub[0], (int)ins->from_match.sub[1]);
			ins->feed=ins->in_buf + (int)ins->from_match.sub[1];
			ins->feed_len=ins->in_len - (int)ins->from_match.sub[1];
			ins->from_match.sub[0]=(struct state_s *) ins->in_buf;
		}else{
			memcpy(ins->in_buf, ins->feed, ins->feed_len);
			ins->feed=ins->in_buf + ins->feed_len;
			ins->feed_len=ins->in_len - ins->feed_len;
		}
		return 1;
}
