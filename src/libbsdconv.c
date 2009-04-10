#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <bsdconv.h>

struct bsdconv_codec_t {
	char *desc;
	int fd;
	void *z;
}

struct bsdconv_t {
	int nfrom;
	int ninter;
	int nto;
	bsdconv_codec_t from[];
	bsdconv_codec_t inter[];
	bsdconv_codec_t to[];
};

#define COUNT(X) do{		\
	if(X){			\
		nX=1;		\
	}else{			\
		nX=0;		\
	}			\
	for(t=X;t;++t){		\
		if(*t==','){	\
			nX++;	\
		}		\
	}			\
	ret->nX=nX;		\
}while(0);

#define GENLIST(X) do{										\
	X=strdup(X);										\
	ret->X=malloc(nX * sizeof(bsdconv_codec_t));						\
	ret->X[0].desc=X;									\
	for(j=0,t=X;t;++t){									\
		if(*t==','){									\
			*t=0;									\
			mkmmap:									\
			ret->X[j].fd=open(ret->X[j].desc, O_RDONLY);				\
			fstat(ret->X[j].fd, &stat);						\
			ret->X[j].z=mmap(0,stat.st_size,PROT_READ|PROT_WRITE,0,ret->X[j].fd,0);	\
			if(j+1 < nX){								\
				ret->X[++j].desc=t+1;						\
			}									\
		}else{										\
			*t=toupper(*t); 							\
		}										\
	}											\
}while(0);

struct bsdconv_t *bsdconv_create(char *from, char *to, char *inter=NULL){
	bsdconv_t *ret=new malloc(sizeof(bsdcont_t));
	struct stat stat;
	char *t;
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
}

bsdconv_destroy(struct bsdconv_t *cd){
	free(cd);
}

bsd_conv(struct bsdconv_t *cd, const char *inbuf, size_t *inlen, char *outbuf, size_t *outlen){
	int from_state=0, inter_state=0, to_state=0;
	int from_index=0, inter_index=0, to_index=0;
	int i;
	char *from_ptr=inbuf;
	struct state_s from_match={.len=0},inter_data;
	struct state_s blackhole={
		.len=0;
	}
	struct state_s terminator={
		.date="\x3f",
		.len=1,
	}

	//from
	phase_from:
	while(from_ptr<inbuf+inlen){
		from_data=*from_ptr;
		from_state=cd->from[from_index].z[from_state]->sub[from_data];
		switch(from_state->status){
			case DEADEND:
				if(from_match.len){
					inter_data=from_match;
					from_match.len=0;
					from_index=0;
					from_state=0;
					++from_ptr;
					goto phase_inter;
				}else if(from_index==cd->nfrom){
					inter_data=terminator;
					from_index=0;
					from_state=0;
					++from_ptr;
				}else{
					from_index++;
					from_state=0;
					++from_ptr;
				}
				break;
			case MATCH:
				from_match.data=from_state->data;
				from_match.len=from_state->len;
				from_match.sub[0]=from_ptr;
				++from_ptr;
				break;
			default:
				++from_ptr;
		}
	}

	//inter
	phase_inter:
	to_data=empty;
	for(i=0;i<data_len;i++){
		inter_state=cd->inter[inter_index].z[inter_state]->sub[inter_data];
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

