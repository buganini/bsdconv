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

#define COUNT(X) do{				\
	if(o##X){				\
		n##X=1;				\
		for(t=(char *)o##X;*t;t++){	\
			if(*t==','){	\
				n##X++;		\
			}			\
		}				\
	}else{					\
		n##X=0;				\
	}					\
	ret->n##X=n##X;				\
}while(0);

#define GENLIST(X) do{										\
	X=strdup(o##X);										\
	ret->X=malloc(n##X * sizeof(struct bsdconv_codec_t));					\
	ret->X[0].desc=X;									\
	for(i=0,t=X;;++t){									\
		if(*t==',' || *t==0){									\
			*t=0;									\
			ret->X[i].fd=open(ret->X[i].desc, O_RDONLY);				\
			fstat(ret->X[i].fd, &stat);						\
			ret->X[i].z=mmap(0,stat.st_size,PROT_READ, MAP_PRIVATE,ret->X[i].fd,0);	\
			if(i+1 < n##X){								\
				ret->X[++i].desc=t+1;						\
			}									\
			if(*t==0){				\
				break;				\
			}					\
		}else{										\
			*t=toupper(*t); 							\
		}										\
	}											\
}while(0);

void bsdconv_init(struct bsdconv_t *cd, struct bsdconv_instruction *ins, unsigned char *inbuf, size_t inlen, unsigned char *outbuf, size_t outlen){
	ins->in_buf=inbuf;
	ins->in_len=inlen;
	ins->out_buf=outbuf;	//*
	ins->out_len=outlen;
	ins->feed=inbuf;
	ins->feed_len=inlen;
	ins->back=outbuf;	//*
	ins->back_len=0;
	/* (*)never changed */

	ins->ierr=0;
	ins->oerr=0;

	ins->from_index=0;
	ins->inter_index=0;
	ins->to_index=0;

	ins->inter_data.data=0;
	ins->to_data.data=0;
	ins->out_data.data=0;

	ins->from_match.data=0;
	ins->inter_match.data=0;
	ins->to_match.data=0;

	ins->from_match.sub[0]=(int)ins->feed;
	ins->from_match.sub[1]=ins->feed_len;

	memcpy(&ins->from_state, cd->from[0].z, sizeof(struct state_s));
	memcpy(&ins->inter_state, cd->inter[0].z, sizeof(struct state_s));
	memcpy(&ins->to_state, cd->to[0].z, sizeof(struct state_s));
}

struct bsdconv_t *bsdconv_create(const char *ofrom, const char *ointer, const char *oto){
	struct bsdconv_t *ret=malloc(sizeof(struct bsdconv_t));
	struct stat stat;
	char *t, *from, *inter, *to;
	int i,nfrom,nto,ninter;

	COUNT(from);
	COUNT(inter);
	COUNT(to);
	chdir("/usr/local/share/bsdconv/codecs");
	if(nfrom==0 || nto==0 || ninter==0){
		fprintf(stderr, "Need at least 1 from and to encoding.\n Use \"dummy\" for none inter-map.");
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
	free(cd);
}

int bsd_conv(struct bsdconv_t *cd, struct bsdconv_instruction *ins){
	unsigned int i;
	struct state_s tmp_state;

	struct data_s iterminator={
		.data=(unsigned int)"\x01\x3f",
		.len=2,
		.next=0,
	};
	iterminator.p=(unsigned int)&iterminator;
	struct data_s oterminator={
		.data=(unsigned int)"?",
		.len=1,
		.next=0,
	};
	oterminator.p=(unsigned int)&oterminator;

	ins->back_len=0;

	ins->feed_len+=ins->feed - ins->in_buf;
	ins->feed=ins->in_buf;

	if(ins->out_data.data){
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

	//from
	phase_from:
	while(ins->feed_len){
		memcpy(&ins->from_state, cd->from[ins->from_index].z + ins->from_state.sub[*ins->feed], sizeof(struct state_s));
		--ins->feed_len;
		++ins->feed;
		switch(ins->from_state.status){
			case DEADEND:
				pass_to_inter:
				ins->pend_from=0;
				if(ins->from_match.data){
					memcpy(&ins->inter_data, ins->inter_z + ins->from_match.data, sizeof(struct data_s));
					ins->inter_d=ins->inter_data.data+ins->inter_z;
					ins->from_match.data=0;
					memcpy(&ins->inter_state, cd->inter[0].z, sizeof(struct state_s));
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));
					ins->feed=(unsigned char *)ins->from_match.sub[0];
					ins->feed_len=ins->from_match.sub[1];
					ins->from_index=0;
					ins->from_match.sub[0]=(unsigned int)ins->feed;
					ins->from_match.sub[1]=ins->feed_len;
					goto phase_inter;
				}else if(ins->from_index < cd->nfrom){
					ins->from_index++;
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));
					ins->feed=(unsigned char *)ins->from_match.sub[0];
					ins->feed_len=ins->from_match.sub[1];
					continue;
				}else{
					ins->from_index=0;
					ins->ierr++;
					ins->inter_data=iterminator;
					ins->inter_z=0;
					memcpy(&ins->from_state, cd->from[ins->from_index].z, sizeof(struct state_s));
					ins->from_match.sub[0]=(unsigned int)ins->feed;
					ins->from_match.sub[1]=ins->feed_len;
					goto phase_inter;
				}
				break;
			case MATCH:
				ins->from_match.data=ins->from_state.data;
				ins->inter_z=cd->from[ins->from_index].z;
				ins->from_match.sub[0]=(unsigned int)ins->feed;
				ins->from_match.sub[1]=ins->feed_len;
				break;
			case CONTINUE:
				ins->pend_from=1;
		}
	}

	//inter
	phase_inter:
	ins->inter_match.sub[0]=ins->inter_data.p + (unsigned int)ins->inter_z;
	while(ins->inter_data.data){
		for(i=0;i<ins->inter_data.len;i++){
			memcpy(&ins->inter_state, cd->inter[ins->inter_index].z + ins->inter_state.sub[ins->inter_d[i]], sizeof(struct state_s));
			if(ins->inter_state.status==DEADEND){
				break;
			}
		}
		i=ins->inter_state.status;
		ins->to_data=ins->inter_data;
		ins->to_data.next=0;
		ins->to_z=ins->inter_z;
		ins->to_d=ins->to_data.data+ins->to_z;
		if(ins->inter_data.next){
			memcpy(&ins->inter_state, cd->inter[ins->inter_index].z + ins->inter_state.sub[256], sizeof(struct state_s));
			memcpy(&ins->inter_data, ins->inter_z + ins->inter_data.next, sizeof(struct data_s));
			ins->inter_d=ins->inter_data.data + ins->inter_z;
		}else{
			ins->inter_data.data=0;
		}
		switch(i){
			case DEADEND:
				pass_to_to:
				ins->pend_inter=0;
				if(ins->inter_match.data){
					memcpy(&ins->to_data, ins->to_z + ins->inter_match.data, sizeof(struct data_s));
					ins->to_d=ins->to_data.data+ins->to_z;
					ins->inter_match.data=0;
					memcpy(&ins->to_state, cd->to[0].z, sizeof(struct state_s));
					memcpy(&ins->inter_data, (char *)ins->inter_match.sub[0], sizeof(struct data_s));
					i=0;
					ins->inter_index=0;
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));
					goto phase_to;
				}else if(ins->inter_index < cd->ninter){
					ins->inter_index++;
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));
					memcpy(&ins->inter_data, (char *)ins->inter_match.sub[0], sizeof(struct data_s));
					i=0;
					continue;
				}else{
					memcpy(&ins->to_data, (char *)ins->inter_match.sub[0], sizeof(struct data_s));
					memcpy(&ins->inter_state, cd->inter[ins->inter_index].z, sizeof(struct state_s));
					ins->inter_index=0;
					goto phase_to;
				}
				break;
			case MATCH:
				ins->inter_match.data=ins->inter_state.data;
				ins->to_z=cd->inter[ins->inter_index].z;
				ins->inter_match.sub[0]=ins->inter_data.p + (unsigned int)ins->inter_z;
				break;
			case CONTINUE:
				ins->pend_inter=1;
		}
	}

	//to
	phase_to:
	ins->to_match.sub[0]=ins->to_data.p + (unsigned int)ins->to_z;
	while(ins->to_data.data){
		for(i=0;i<ins->to_data.len;i++){
			memcpy(&ins->to_state, cd->to[ins->to_index].z + ins->to_state.sub[ins->to_d[i]], sizeof(struct state_s));
			if(ins->to_state.status==DEADEND){
				break;
			}
		}
		i=ins->to_state.status;
		if(ins->to_data.next){
			memcpy(&ins->to_state, cd->to[ins->to_index].z + ins->to_state.sub[256], sizeof(struct state_s));
			memcpy(&ins->to_data, ins->to_z + ins->to_data.next, sizeof(struct data_s));
			ins->to_d=ins->to_data.data + ins->to_z;
		}else{
			ins->to_data.data=0;
		}
		switch(i){
			case DEADEND:
				pass_to_out:
				ins->pend_to=0;
				if(ins->to_match.data){
					memcpy(&ins->out_data, ins->out_z + ins->to_match.data, sizeof(struct data_s));
					ins->out_d=ins->out_data.data+ins->out_z;
					ins->to_match.data=0;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));
					memcpy(&ins->to_data, (char *)ins->to_match.sub[0], sizeof(struct data_s));
					i=0;
					ins->to_index=0;
					goto phase_out;
				}else if(ins->to_index < cd->nto){
					ins->to_index++;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));
					memcpy(&ins->to_data, (char *)ins->to_match.sub[0], sizeof(struct data_s));
					i=0;
					continue;
				}else{
					ins->to_index=0;
					ins->oerr++;
					ins->out_data=oterminator;
					ins->out_z=0;
					ins->out_d=(unsigned char *)ins->out_data.data;
					memcpy(&ins->to_state, cd->to[ins->to_index].z, sizeof(struct state_s));
					goto phase_out;
				}
				break;
			case MATCH:
				ins->to_match.data=ins->to_state.data;
				ins->out_z=cd->to[ins->to_index].z;
				ins->to_match.sub[0]=ins->to_data.p + (unsigned int)ins->to_z;
				break;
			case CONTINUE:
				ins->pend_to=1;
		}
	}

	//out
	phase_out:
	while(ins->out_data.data){
		i=ins->back_len + ins->out_data.len;
		if(i > ins->out_len){
			goto hibernate;
		}else{
			memcpy(ins->back + ins->back_len, ins->out_d, ins->out_data.len);
			ins->back_len=i;
		}
		if(ins->out_data.next){
			memcpy(&ins->out_data, ins->out_z + ins->out_data.next, sizeof(struct data_s));
			ins->out_d=ins->out_data.data + ins->out_z;
		}else{
			ins->out_data.data=0;
		}
	}

	if(ins->to_data.data){
		goto phase_to;
	}
	if(ins->inter_data.data){
		goto phase_inter;
	}
	if(ins->feed_len){
		goto phase_from;
	}

	return 0;

	hibernate:
		memcpy(ins->in_buf, ins->feed, ins->feed_len);
		ins->feed=ins->in_buf + ins->feed_len;
		ins->feed_len=ins->in_len - ins->feed_len;
		return 1;
}
