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
	int filter;
	int mode;
};

void cbcreate(struct bsdconv_instance *ins, struct hash_entry *arg){
	struct my_s *r=malloc(sizeof(struct my_s));
	CURRENT_CODEC(ins)->priv=r;
	r->filter=1;
	r->mode=16;
	r->prefix.data=strdup("%");
	r->prefix.len=1;
	r->suffix.data=strdup("");
	r->suffix.len=0;
	while(arg){
		if(strcmp(arg->key, "PREFIX")==0){
			free(r->prefix.data);
			str2data(arg->ptr, &(r->prefix));
		}else if(strcmp(arg->key, "SUFFIX")==0){
			free(r->suffix.data);
			str2data(arg->ptr, &(r->suffix));
		}else if(strcmp(arg->key, "MODE")==0){
			if(strcmp(arg->ptr, "HEX")==0 || strcmp(arg->ptr, "16")==0){
				r->mode=16;
			}else if(strcmp(arg->ptr, "DEC")==0 || strcmp(arg->ptr, "10")==0){
				r->mode=10;
			}else if(strcmp(arg->ptr, "OCT")==0 || strcmp(arg->ptr, "8")==0){
				r->mode=8;
			}
		}else if(strcmp(arg->key, "FILTER")==0){
			if(strcmp(arg->ptr, "UNICODE")==0 || strcmp(arg->ptr, "1")==0 || strcmp(arg->ptr, "01")==0){
				r->filter=1;
			}else if(strcmp(arg->ptr, "BYTE")==0 || strcmp(arg->ptr, "3" || strcmp(arg->ptr, "03")==0)==0){
				r->filter=3;
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

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *t=CURRENT_CODEC(ins)->priv;
	int i;
	unsigned int u;
	char *p;

	if(t->filter==1 && this_phase->curr->len>1 && UCP(this_phase->curr->data)[0]==1){ //unicode
		if(t->mode==16){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=(this_phase->curr->len-1)*2+t->prefix.len + t->suffix.len;
			this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
			memcpy(this_phase->data_tail->data, t->prefix.data, t->prefix.len);
			p=this_phase->data_tail->data+t->prefix.len;

			for(i=1;i<this_phase->curr->len;++i){
				p+=sprintf(p,"%02X", UCP(this_phase->curr->data)[i]);
			}
			memcpy(p, t->suffix.data, t->suffix.len);
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else if(t->mode==10){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=(this_phase->curr->len-1)*3+t->prefix.len + t->suffix.len;
			this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
			memcpy(this_phase->data_tail->data, t->prefix.data, t->prefix.len);
			p=this_phase->data_tail->data+t->prefix.len;

			u=0;
			for(i=1;i<this_phase->curr->len;++i){
				u*=256;
				u+=UCP(this_phase->curr->data)[i];
			}
			p+=sprintf(p, "%u", u);
			memcpy(p, t->suffix.data, t->suffix.len);
			this_phase->data_tail->len=(p+t->suffix.len)-CP(this_phase->data_tail->data);
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else{
			ins->phase[ins->phase_index].state.status=DEADEND;
		}
	}else if(t->filter==3 && this_phase->curr->len==2 && UCP(this_phase->curr->data)[0]==3){ //byte
		if(t->mode==8){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=3+t->prefix.len + t->suffix.len;
			this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
			memcpy(this_phase->data_tail->data, t->prefix.data, t->prefix.len);
			p=this_phase->data_tail->data+t->prefix.len;
			i=UCP(this_phase->curr->data)[1];
			*UCP(p+2)=i%8+'0';
			i/=8;
			*UCP(p+1)=i%8+'0';
			i/=8;
			*UCP(p)=i+'0';
			memcpy(p+3, t->suffix.data, t->suffix.len);
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else if(t->mode==10){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=3+t->prefix.len + t->suffix.len;
			this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
			memcpy(this_phase->data_tail->data, t->prefix.data, t->prefix.len);
			p=this_phase->data_tail->data+t->prefix.len;
			p+=sprintf(p, "%d", UCP(this_phase->curr->data)[1]);
			memcpy(p, t->suffix.data, t->suffix.len);
			this_phase->data_tail->len=(p+t->suffix.len)-CP(this_phase->data_tail->data);
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else if(t->mode==16){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->flags=F_FREE;
			this_phase->data_tail->len=2+t->prefix.len + t->suffix.len;
			this_phase->data_tail->data=malloc(this_phase->data_tail->len+1);
			memcpy(this_phase->data_tail->data, t->prefix.data, t->prefix.len);
			p=this_phase->data_tail->data+t->prefix.len;
			p+=sprintf(p, "%02X", UCP(this_phase->curr->data)[1]);
			memcpy(p, t->suffix.data, t->suffix.len);
			ins->phase[ins->phase_index].state.status=NEXTPHASE;
		}else{
			ins->phase[ins->phase_index].state.status=DEADEND;
		}
	}else{
		ins->phase[ins->phase_index].state.status=DEADEND;
	}

	return;
}
