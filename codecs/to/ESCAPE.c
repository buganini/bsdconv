/*
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

struct my_s{
	struct data_st prefix;
	struct data_st suffix;
	int mode;
};

void cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->mode=16;
	r->prefix.data=strdup("%");
	r->prefix.len=1;
	r->suffix.data=strdup("");
	r->suffix.len=0;
	while(arg){
		if(strcmp(arg->key, "PREFIX")==0){
			str2data(arg->ptr, &(r->prefix));
		}else if(strcmp(arg->key, "SUFFIX")==0){
			str2data(arg->ptr, &(r->suffix));
		}else if(strcmp(arg->key, "MODE")==0){
			if(strcmp(arg->ptr, "HEX")==0 || strcmp(arg->ptr, "16")==0){
				r->mode=16;
			}else if(strcmp(arg->ptr, "OCT")==0 || strcmp(arg->ptr, "8")==0){
				r->mode=8;
			}else if(strcmp(arg->ptr, "UNICODE")==0){
				r->mode=0;
			}
		}
		arg=arg->next;
	}
}
void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	free(r->prefix.data);
	free(r->suffix.data);
	free(r);
}


#define TAILIZE(p) while(*p){ p++ ;}

#define prefix() do{ \
	if(*(t->prefix.data)){ \
		DATA_MALLOC(this_phase->data_tail->next); \
		this_phase->data_tail=this_phase->data_tail->next; \
		this_phase->data_tail->next=NULL; \
		this_phase->data_tail->flags=0; \
		\
		this_phase->data_tail->len=t->prefix.len; \
		this_phase->data_tail->data=t->prefix.data; \
	} \
}while(0)

#define suffix() do{ \
	if(*(t->suffix.data)){ \
		DATA_MALLOC(this_phase->data_tail->next); \
		this_phase->data_tail=this_phase->data_tail->next; \
		this_phase->data_tail->next=NULL; \
		this_phase->data_tail->flags=0; \
		\
		this_phase->data_tail->len=t->suffix.len; \
		this_phase->data_tail->data=t->suffix.data; \
	} \
}while(0)

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	int i;
	char *p;

	if(this_phase->curr->len>1 && UCP(this_phase->curr->data)[0]==1 && t->mode==0){ //unicode
		prefix();
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->flags=F_FREE;

		this_phase->data_tail->len=(this_phase->curr->len-1)*2;
		p=this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
		for(i=1;i<this_phase->curr->len;++i){
			sprintf(p,"%02X", UCP(this_phase->curr->data)[i]);
			TAILIZE(p);
		}
		suffix();
		ins->phase[ins->phase_index].state.status=NEXTPHASE;
	}else if(this_phase->curr->len==2 && UCP(this_phase->curr->data)[0]==3){ //byte
		if(t->mode==8){
			prefix();
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=3;
			this_phase->data_tail->data=malloc(3);
			i=UCP(this_phase->curr->data)[1];
			UCP(this_phase->data_tail->data)[2]=i%8+'0';
			i/=8;
			UCP(this_phase->data_tail->data)[1]=i%8+'0';
			i/=8;
			UCP(this_phase->data_tail->data)[0]=i+'0';
			suffix();
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else if(t->mode==16){
			prefix();
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=2;
			this_phase->data_tail->data=malloc(3);
			sprintf(this_phase->data_tail->data, "%02X", UCP(this_phase->curr->data)[1]);
			suffix();
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else{
			ins->phase[ins->phase_index].state.status=DEADEND;
		}
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
	}

	return;
}
