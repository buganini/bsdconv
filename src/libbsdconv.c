#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include "bsdconv.h"

struct bsdconv_codec_t {
	char *desc;
	int fd;
	unsigned char *z;
};

struct bsdconv_t {
	int nfrom;
	int ninter;
	int nto;
	struct bsdconv_codec_t *from;
	struct bsdconv_codec_t *inter;
	struct bsdconv_codec_t *to;
};

#define COUNT(X) do{			\
	if(o##X){			\
		n##X=1;			\
	}else{				\
		n##X=0;			\
	}				\
	for(t=(char *)o##X;t;++t){		\
		if(*t==','){		\
			n##X++;		\
		}			\
	}				\
	ret->n##X=n##X;			\
}while(0);

#define GENLIST(X) do{										\
	X=strdup(o##X);										\
	ret->X=malloc(n##X * sizeof(struct bsdconv_codec_t));					\
	ret->X[0].desc=X;									\
	for(j=0,t=X;t;++t){									\
		if(*t==','){									\
			*t=0;									\
			ret->X[j].fd=open(ret->X[j].desc, O_RDONLY);				\
			fstat(ret->X[j].fd, &stat);						\
			ret->X[j].z=mmap(0,stat.st_size,PROT_READ|PROT_WRITE,0,ret->X[j].fd,0);	\
			if(j+1 < n##X){								\
				ret->X[++j].desc=t+1;						\
			}									\
		}else{										\
			*t=toupper(*t); 							\
		}										\
	}											\
}while(0);

struct bsdconv_t *bsdconv_create(const char *ofrom, const char *oto, const char *ointer){
	struct bsdconv_t *ret=malloc(sizeof(struct bsdconv_t));
	struct stat stat;
	char *t, *from, *inter, *to;
	int i,j,nfrom,nto,ninter;
	COUNT(from);
	COUNT(inter);
	COUNT(to);
	chdir("/usr/local/share/bsdconv/codecs");
	if(nfrom==0 || nto==0){
		fprintf(stderr, "Need at least 1 from and to encoding.\n");
		return NULL;
	}
	GENLIST(from);
	GENLIST(to);
	if(ninter){
		ret->inter=NULL;
	}else{
		GENLIST(inter);
	}
	return ret;
}

bsdconv_destroy(struct bsdconv_t *cd){
	free(cd);
}

bsd_conv(struct bsdconv_t *cd, const char *inbuf, size_t *inlen, char *outbuf, size_t *outlen){
	int from_state=0, inter_state=0, to_state=0;
	int from_index=0, inter_index=0, to_index=0;
	unsigned char from_data;
	int i;
	char *from_ptr=(char *) inbuf;
	struct state_s state_tmp, from_match;
	struct data_s data_tmp, inter_data, to_data;
	struct state_s blackhole={
		.data=0,
	};
	struct data_s terminator={
		.data=(int)"\x01\x3f",
		.len=2,
	};

	from_match.sub[0]=(int)from_ptr;
	//from
	phase_from:
	while(from_ptr < inbuf+*inlen){
		from_data=*from_ptr;
		memcpy(&state_tmp, cd->from[from_index].z + from_state, sizeof(struct state_s));
		from_state=(int)state_tmp.sub[from_data];
		memcpy(&state_tmp, cd->from[from_index].z + from_state, sizeof(struct state_s));
		switch(state_tmp.status){
			case DEADEND:
				if(from_match.data){
					memcpy(&data_tmp, cd->from[from_index].z + from_match.data, sizeof(struct data_s));
					inter_data.data=data_tmp.data;
					inter_data.len=data_tmp.len;
					from_match.data=0;
					from_index=0;
					from_state=0;
					from_ptr=(char *)from_match.sub[0];
					goto phase_inter;
				}else if(from_index==cd->nfrom){
					inter_data.data=terminator.data;
					inter_data.len=terminator.len;
					from_index=0;
					from_state=0;
					++from_ptr;
				}else{
					from_index++;
					from_state=0;
					from_ptr=(char *)from_match.sub[0];
				}
				break;
			case MATCH:
				from_match.data=(int)cd->from[from_index].z + state_tmp.data;
				from_match.sub[0]=(int)from_ptr;
				++from_ptr;
				break;
			default:
				++from_ptr;
		}
	}

	//inter
	phase_inter:
	for(i=0;i<inter_data->len;i++){
		memcpy(&tmp, &(cd->inter[inter_index].z[inter_state]), sizeof(struct state_s));
		from_state=tmp.sub[inter_data];
		memcpy(&tmp, &(cd->inter[inter_index].z[inter_state]), sizeof(struct state_s));
		switch(tmp.status){
			case DEADEND:
			case MATCH:
			default:
		}
		if(inter_state->len){
			has_match=1;
		}else if(has_match){
			has_match=0;
			to_date=last_match;
			inter_index=0;
			inter_state=0;
		}else if(inter_index==index_index_end){
			to_date=inter_data;
		}else{
			inter_index++;
		}
	}

	//to
	phase_to:
	while(to_data && to_continue){
		to_state=cd->to[to_index].z[to_state]->sub[to_data];
		if(to_state->len){
			out_data=
		}else if(has_match){
		}elseif (to_index==to_index_end){
			
		}else{
		
		}
	}

	if(from_ptr<inbuf+inlen){
		goto phase_from;
	}
	if(from_match.len){
		inter_data=from_match;
		from_match.len=0;
		goto phase_inter;
	}
	
}

