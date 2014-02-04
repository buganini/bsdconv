#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	FILE *bak;
	FILE *score;
	struct bsdconv_scorer *scorer;
	bsdconv_counter_t *counter;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	char buf[256]={0};
	r->bak=NULL;
	r->score=NULL;
	r->scorer=NULL;

	char *scorer="CJK";
	char *counter="SCORE";
	while(arg){
		if(strcasecmp(arg->key, "WITH")==0){
			scorer=arg->ptr;
		}else if(strcasecmp(arg->key, "AS")==0){
			counter=arg->ptr;
		}else{
			return EINVAL;
		}
		arg=arg->next;
	}

	if(strcasecmp(scorer, "TRAINED")==0){
		char *p=getenv("BSDCONV_SCORE");
		if(p==NULL){
			//try default score file
			strcpy(buf,getenv("HOME"));
			strcat(buf,"/.bsdconv.score");
			p=buf;
		}
		r->bak=r->score=fopen(p,"rb+");
	}else{
		r->scorer=load_scorer(scorer);
		if(r->scorer==NULL){
			free(r);
			return EOPNOTSUPP;
		}
	}

	r->counter=bsdconv_counter(ins, counter);
	THIS_CODEC(ins)->priv=r;
	return 0;
}

void cbctl(struct bsdconv_instance *ins, int ctl, void *ptr, size_t v){
	struct my_s *r=THIS_CODEC(ins)->priv;
	switch(ctl){
		case BSDCONV_CTL_ATTACH_SCORE:
			r->score=ptr;
			break;
	}
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	if(r->bak)
		fclose(r->bak);
	if(r->scorer)
		unload_scorer(r->scorer);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;
	FILE *fp=r->score;
	int i;
	uint32_t ucs=0;
	unsigned char v;
	uint32_t score;
	data=this_phase->curr->data;

	if(r->scorer!=NULL){
		score=r->scorer->cbscorer(this_phase->curr);
		*(r->counter)+=score;
		if(score==0){
			this_phase->state.status=DEADEND;
			return;
		}
	}else if(fp!=NULL && this_phase->curr->len>0 && UCP(this_phase->curr->data)[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		fseek(fp, ucs*sizeof(unsigned char), SEEK_SET);
		fread(&v, sizeof(unsigned char), 1, fp);
		*(r->counter)+=v;
	}

	this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	this_phase->state.status=NEXTPHASE;
	return;
}
