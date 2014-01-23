void bsdconv_hash_set(struct bsdconv_instance *ins, const char *key, void *ptr){
	char *tk;
	void *tp;
	struct bsdconv_hash_entry *p=ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			tp=ptr;
			tk=p->key;
			p->key=ins->hash->key;
			p->ptr=ins->hash->ptr;
			ins->hash->key=tk;
			ins->hash->ptr=tp;
			return;
		}
		p=p->next;
	}
	p=malloc(sizeof(struct bsdconv_hash_entry));
	p->next=ins->hash;
	ins->hash=p;
	p->key=strdup(key);
	p->ptr=ptr;
	return;
}

void *bsdconv_hash_get(struct bsdconv_instance *ins, const char *key){
	char *tk;
	void *tp;
	struct bsdconv_hash_entry *p=ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			tk=p->key;
			tp=p->ptr;
			p->key=ins->hash->key;
			p->ptr=ins->hash->ptr;
			ins->hash->key=tk;
			ins->hash->ptr=tp;
			return p->ptr;
		}
		p=p->next;
	}
	return NULL;
}

int bsdconv_hash_has(struct bsdconv_instance *ins, const char *key){
	char *tk;
	void *tp;
	struct bsdconv_hash_entry *p=ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			tk=p->key;
			tp=p->ptr;
			p->key=ins->hash->key;
			p->ptr=ins->hash->ptr;
			ins->hash->key=tk;
			ins->hash->ptr=tp;
			return 1;
		}
		p=p->next;
	}
	return 0;
}

void bsdconv_hash_del(struct bsdconv_instance *ins, const char *key){
	struct bsdconv_hash_entry *p=ins->hash;
	struct bsdconv_hash_entry **q=&ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			free(p->key);
			*q=p->next;
			free(p);
			return;
		}
		p=p->next;
		q=&p->next;
	}
}
