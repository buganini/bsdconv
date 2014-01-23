/*
 * Copyright (c) 2009-2014 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

bsdconv_counter_t * bsdconv_counter(struct bsdconv_instance *ins, const char *_key){
	struct bsdconv_counter_entry *p=ins->counter;
	struct bsdconv_counter_entry *t;
	char *key=strdup(_key);
	strtoupper(key);
	if(p==NULL){
		ins->counter=malloc(sizeof(struct bsdconv_counter_entry));
		ins->counter->key=key;
		ins->counter->val=0;
		ins->counter->next=0;
		return &ins->counter->val;
	}else{
		do{
			t=p;
			if(strcmp(p->key, key)==0){
				free(key);
				return &p->val;
			}
			p=p->next;
		}while(p!=NULL);
		t->next=malloc(sizeof(struct bsdconv_counter_entry));
		t=t->next;
		t->key=key;
		t->val=0;
		t->next=0;
		return &t->val;
	}
}

void bsdconv_counter_reset(struct bsdconv_instance *ins, const char *key){
	struct bsdconv_counter_entry *p=ins->counter;
	bsdconv_counter_t *v;
	if(key==NULL){
		while(p){
			p->val=0;
			p=p->next;
		}
	}else{
		v=bsdconv_counter(ins, key);
		*v=0;
	}
}
