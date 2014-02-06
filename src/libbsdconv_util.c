void *bsdconv_malloc(size_t size){
	return malloc(size);
}

void bsdconv_free(void *p){
	free(p);
}

int bsdconv_mkstemp(char *template){
	return mkstemp(template);
}

char * getCodecDir(){
	char *c;
	char *b;
	if((c=getenv("BSDCONV_PATH"))==NULL){
		c=BSDCONV_PATH;
	}
	b=malloc(strlen(c)+strlen(MODULES_SUBPATH)+2);
	sprintf(b, "%s/%s", c, MODULES_SUBPATH);
	return b;
}

int str2datum(const char *s, struct data_rt *d){
	d->data=NULL;
	d->len=0;
	if(!s)
		return EINVAL;
	d->data=malloc(strlen(s)/2);
	d->flags=F_FREE;
	d->next=NULL;
	char f=0;
	while(*s){
		if(hex[(unsigned char) *s]<0){
			free(d->data);
			d->data=NULL;
			return EINVAL;
		}
		switch(f){
			case 0:
				f=1;
				UCP(d->data)[d->len]=hex[(unsigned char)*s];
				break;
			case 1:
				f=0;
				UCP(d->data)[d->len]*=16;
				UCP(d->data)[d->len]+=hex[(unsigned char)*s];
				d->len+=1;
				break;
		}
		s+=1;
	}
	return 0;
}

struct data_rt * str2data(const char *_s, int *r, struct bsdconv_instance *ins){
	struct data_rt ph;
	struct data_rt *t=&ph;
	char *k, *cur;
	char *s;
	char f;

	ph.next=NULL;
	if(!_s){
		*r=EINVAL;
		return NULL;
	}
	if(!*_s){
		*r=0;
		return NULL;
	}

	s=strdup(_s);

	cur=s;
	while((k=strsep(&cur, "."))!=NULL){
		t->next=malloc(sizeof(struct data_rt));
		t=t->next;
		t->next=NULL;
		t->len=0;
		t->flags=F_FREE;
		t->data=malloc(strlen(k)/2);
		f=0;
		while(*k){
			if(hex[(unsigned char) *k]<0){
				DATA_FREE(ins, ph.next);
				*r=EINVAL;
				free(s);
				return NULL;
			}
			switch(f){
				case 0:
					f=1;
					UCP(t->data)[t->len]=hex[(unsigned char)*k];
					break;
				case 1:
					f=0;
					UCP(t->data)[t->len]*=16;
					UCP(t->data)[t->len]+=hex[(unsigned char)*k];
					t->len+=1;
					break;
			}
			k+=1;
		}
	}

	free(s);

	*r=0;
	return ph.next;
}

int bsdconv_get_phase_index(struct bsdconv_instance *ins, int phasen){
	/*
	 * phase[0] is a place holder for _INPUT
	 * real phases range is [1,len]=[1,phasen]
	 */
	/* logical new index = len */
	if(phasen /* logical */ >= ins->phasen /* len */){
		/* real  = logical + 1 */
		return ins->phasen + 1;
	}else{
		/* real  = (n + len) % (len) + 1*/
		return (phasen + ins->phasen) % (ins->phasen) + 1;
	}
}

int bsdconv_get_codec_index(struct bsdconv_instance *ins, int phasen, int codecn){
	/*
	 * codecn is -=1 for convenient use as boundary
	 * real phases range is [0,len)=[0,codecn]
	 */
	phasen=bsdconv_get_phase_index(ins, phasen);

	/* logical new index = len */
	if(codecn /* logical */ >= ins->phase[phasen].codecn+1 /* len */ ){
		/* real  = logical */
		return ins->phase[phasen].codecn+1;
	}else{
		/* real  = (n + len) % (len) */
		return (codecn + ins->phase[phasen].codecn+1) % (ins->phase[phasen].codecn+1);
	}
}

char * bsdconv_insert_phase(const char *conversion, const char *codec, int phase_type, int ophasen){
	struct bsdconv_instance *ins;
	int i,j;
	char *ret;

	ins=bsdconv_unpack(conversion);
	if(!ins){
		return NULL;
	}

	int phasen=bsdconv_get_phase_index(ins, ophasen);

	ins->phasen+=1;
	ins->phase=realloc(ins->phase, sizeof(struct bsdconv_phase) * (ins->phasen+1));

	for(i=ins->phasen /* shifted index */;i>phasen;--i){
		ins->phase[i]=ins->phase[i-1];
	}
	ins->phase[phasen].type=phase_type;
	ins->phase[phasen].codec=malloc(sizeof(struct bsdconv_codec));
	ins->phase[phasen].codecn=0 /* trimmed length */;

	ins->phase[phasen].codec[0].desc=strdup(codec);
	ins->phase[phasen].codec[0].argv=NULL;

	ret=bsdconv_pack(ins);

	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			free(ins->phase[i].codec[j].desc);
		}
		free(ins->phase[i].codec);
	}
	free(ins->phase);
	free(ins);

	return ret;
}

char * bsdconv_insert_codec(const char *conversion, const char *codec, int ophasen, int ocodecn){
	struct bsdconv_instance *ins;
	int i,j;
	char *ret;

	ins=bsdconv_unpack(conversion);
	if(!ins){
		return NULL;
	}

	int phasen=bsdconv_get_phase_index(ins, ophasen);
	int codecn=bsdconv_get_codec_index(ins, ophasen, ocodecn);

	++ins->phase[phasen].codecn;
	ins->phase[phasen].codec=realloc(ins->phase[phasen].codec, sizeof(struct bsdconv_codec)*(ins->phase[phasen].codecn+1));

	for(i=ins->phase[phasen].codecn;i>codecn;--i){
		ins->phase[phasen].codec[i]=ins->phase[phasen].codec[i-1];
	}
	ins->phase[phasen].codec[codecn].desc=strdup(codec);
	ins->phase[phasen].codec[codecn].argv=NULL;

	ret=bsdconv_pack(ins);

	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			free(ins->phase[i].codec[j].desc);
		}
		free(ins->phase[i].codec);
	}
	free(ins->phase);
	free(ins);

	return ret;
}

char * bsdconv_replace_phase(const char *conversion, const char *codec, int phase_type, int ophasen){
	struct bsdconv_instance *ins;
	int i,j;
	char *ret;

	ins=bsdconv_unpack(conversion);
	if(!ins){
		return NULL;
	}

	int phasen=bsdconv_get_phase_index(ins, ophasen);

	for(j=0;j<=ins->phase[phasen].codecn;++j){
		free(ins->phase[phasen].codec[j].desc);
	}

	ins->phase[phasen].type=phase_type;
	ins->phase[phasen].codecn=0 /* trimmed length */;
	ins->phase[phasen].codec[0].desc=strdup(codec);
	ins->phase[phasen].codec[0].argv=NULL;

	ret=bsdconv_pack(ins);

	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			free(ins->phase[i].codec[j].desc);
		}
		free(ins->phase[i].codec);
	}
	free(ins->phase);
	free(ins);

	return ret;
}

char * bsdconv_replace_codec(const char *conversion, const char *codec, int ophasen, int ocodecn){
	struct bsdconv_instance *ins;
	int i,j;
	char *ret;

	ins=bsdconv_unpack(conversion);
	if(!ins){
		return NULL;
	}

	int phasen=bsdconv_get_phase_index(ins, ophasen);
	int codecn=bsdconv_get_codec_index(ins, ophasen, ocodecn);

	free(ins->phase[phasen].codec[codecn].desc);
	ins->phase[phasen].codec[codecn].desc=strdup(codec);
	ins->phase[phasen].codec[codecn].argv=NULL;

	ret=bsdconv_pack(ins);

	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			free(ins->phase[i].codec[j].desc);
		}
		free(ins->phase[i].codec);
	}
	free(ins->phase);
	free(ins);

	return ret;
}
