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
	}else{					\
		n##X=0;				\
	}					\
	for(t=(char *)o##X;t;++t){		\
		if(*t==','){			\
			n##X++;			\
		}				\
	}					\
	ret->n##X=n##X;				\
}while(0);

#define GENLIST(X) do{										\
	X=strdup(o##X);										\
	ret->X=malloc(n##X * sizeof(struct bsdconv_codec_t));					\
	ret->X[0].desc=X;									\
	for(i=0,t=X;t;++t){									\
		if(*t==','){									\
			*t=0;									\
			ret->X[i].fd=open(ret->X[i].desc, O_RDONLY);				\
			fstat(ret->X[i].fd, &stat);						\
			ret->X[i].z=mmap(0,stat.st_size,PROT_READ|PROT_WRITE,0,ret->X[i].fd,0);	\
			if(i+1 < n##X){								\
				ret->X[++i].desc=t+1;						\
			}									\
		}else{										\
			*t=toupper(*t); 							\
		}										\
	}											\
}while(0);

void bsdconv_init(struct bsdconv_t *cd, struct bsdconv_instruction *ins, char *inbuf, size_t inlen, char *outbuf, size_t outlen){
	ins->in_buf=inbuf;
	ins->in_len=inlen;
	ins->out_buf=outbuf;	//*
	ins->out_len=outlen;
	ins->feed=inbuf;
	ins->feed_len=inlen;
	ins->back=outbuf;	//*
	ins->back_len=0;
	/* (*)never changed */
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
	if(nfrom==0 || nto==0 || ninter){
		fprintf(stderr, "Need at least 1 from and to encoding.\n Use \"dummy\" for none inter-map.");
		return NULL;
	}
	GENLIST(from);
	GENLIST(inter);
	GENLIST(to);
	return ret;
}

void bsdconv_destroy(struct bsdconv_t *cd){
	free(cd);
}

int bsd_conv(struct bsdconv_t *cd, struct bsdconv_instruction *ins){
	struct state_s from_state, inter_state, to_state;
	int from_index=0, inter_index=0, to_index=0;
	unsigned char *inter_d, *to_d, *out_d, *inter_z, *to_z, *out_z;
	unsigned char from_data;
	int i;
	char *from_ptr=ins->in_buf;
	struct state_s from_match, inter_match, to_match;
	struct data_s inter_data, to_data, out_data;
	struct data_s iterminator={
		.data=(int)"\x01\x3f",
		.len=2,
		.next=0,
	};
	struct data_s oterminator={
		.data=(int)"?",
		.len=1,
		.next=0,
	};

	memcpy(&from_state, cd->from[0].z + 0, sizeof(struct state_s));
	memcpy(&inter_state, cd->inter[0].z + 0, sizeof(struct state_s));
	memcpy(&to_state, cd->to[0].z + 0, sizeof(struct state_s));

	//from
	phase_from:
	from_index=0;
	from_match.sub[0]=(int)from_ptr;
	while(from_ptr < ins->in_buf+ins->in_len){
		from_data=*from_ptr;
		memcpy(&from_state, cd->from[from_index].z + from_state.sub[from_data], sizeof(struct state_s));
		switch(from_state.status){
			case DEADEND:
				if(from_match.data){
					pass_to_inter:
					memcpy(&inter_data, inter_z + from_match.data, sizeof(struct data_s));
					inter_d=inter_data.data+inter_z;
					from_match.data=0;
					memcpy(&from_state, cd->from[from_index].z, sizeof(struct state_s));
					from_ptr=(char *)from_match.sub[0];
					goto phase_inter;
				}else if(from_index==cd->nfrom){
					inter_data=iterminator;
					memcpy(&from_state, cd->from[from_index].z, sizeof(struct state_s));
					goto phase_inter;
				}else{
					from_index++;
					memcpy(&from_state, cd->from[from_index].z, sizeof(struct state_s));
					from_ptr=(char *)from_match.sub[0];
					continue;
				}
				break;
			case MATCH:
				from_match.data=from_state.data;
				inter_z=cd->from[from_index].z;
				from_match.sub[0]=(int)from_ptr;
				break;
		}
		++from_ptr;
	}

	//inter
	phase_inter:
	inter_index=0;
	inter_match.sub[0]=inter_data.p + (int)inter_z;
	while(inter_data.data){
		for(i=0;i<inter_data.len;i++){
			memcpy(&inter_state, cd->inter[inter_index].z + inter_state.sub[inter_d[i]], sizeof(struct state_s));
			if(inter_state.status==DEADEND){
				break;
			}
		}
		switch(inter_state.status){
			case DEADEND:
				if(inter_match.data){
					pass_to_to:
					memcpy(&to_data, to_z + inter_match.data, sizeof(struct data_s));
					to_d=to_data.data+to_z;
					inter_match.data=0;
					memcpy(&inter_state, cd->inter[inter_index].z, sizeof(struct state_s));
					memcpy(&inter_data, (char *)inter_match.sub[0], sizeof(struct data_s));
					i=0;
					goto phase_to;
				}else if(inter_index==cd->ninter){
					to_data=inter_data;
					to_z=inter_z;
					to_d=to_data.data+to_z;
					memcpy(&inter_state, cd->inter[inter_index].z, sizeof(struct state_s));
					goto phase_to;
				}else{
					inter_index++;
					memcpy(&inter_state, cd->inter[inter_index].z, sizeof(struct state_s));
					memcpy(&inter_data, (char *)inter_match.sub[0], sizeof(struct data_s));
					i=0;
					continue;
				}
				break;
			case MATCH:
				inter_match.data=inter_state.data;
				to_z=cd->inter[inter_index].z;
				inter_match.sub[0]=inter_data.p + (int)inter_z;
				break;
		}
		memcpy(&inter_state, cd->inter[inter_index].z + inter_state.sub[256], sizeof(struct state_s));
		if(inter_data.next){
			memcpy(&inter_data, inter_z + inter_data.next, sizeof(struct data_s));
			inter_d=inter_data.data + inter_z;
		}else{
			inter_data.data=0;
		}
	}

	//to
	phase_to:
	to_index=0;
	to_match.sub[0]=to_data.p + (int)to_z;
	while(to_data.data){
		for(i=0;i<to_data.len;i++){
			memcpy(&to_state, cd->to[to_index].z + to_state.sub[to_d[i]], sizeof(struct state_s));
			if(to_state.status==DEADEND){
				break;
			}
		}
		switch(to_state.status){
			case DEADEND:
				if(to_match.data){
					memcpy(&out_data, out_z + to_match.data, sizeof(struct data_s));
					out_d=out_data.data+out_z;
					to_match.data=0;
					memcpy(&to_state, cd->to[to_index].z, sizeof(struct state_s));
					memcpy(&to_data, (char *)to_match.sub[0], sizeof(struct data_s));
					i=0;
					goto phase_out;
				}else if(to_index==cd->nto){
					out_data=oterminator;
					out_z=0;
					out_d=out_data.data+out_z;
					memcpy(&to_state, cd->to[to_index].z, sizeof(struct state_s));
					goto phase_out;
				}else{
					to_index++;
					memcpy(&to_state, cd->to[to_index].z, sizeof(struct state_s));
					memcpy(&to_data, (char *)to_match.sub[0], sizeof(struct data_s));
					i=0;
					continue;
				}
				break;
			case MATCH:
				to_match.data=to_state.data;
				out_z=cd->to[to_index].z;
				to_match.sub[0]=to_data.p + (int)to_z;
				break;
		}
		memcpy(&to_state, cd->to[to_index].z + to_state.sub[256], sizeof(struct state_s));
		if(to_data.next){
			memcpy(&to_data, to_z + to_data.next, sizeof(struct data_s));
			to_d=to_data.data + to_z;
		}else{
			to_data.data=0;
		}
	}

	//out
	phase_out:
	while(out_data.data){
		memcpy(ins->back, to_d, out_data.len);
		if(out_data.next){
			memcpy(&out_data, out_z + to_data.next, sizeof(struct data_s));
			to_d=to_data.data + to_z;
		}else{
			to_data.data=0;
		}
	}

	//repeat and flush
	if(to_data.data)							goto phase_to;
	if(inter_data.data)						goto phase_inter;
	if(from_ptr < ins->in_buf+ins->in_len)	goto phase_from;
	if(from_match.data)						goto pass_to_inter;
	if(inter_match.data)						goto pass_to_to;

//	hibernate:
	return 0;
}

