/*
 * Copyright (c) 2009,2010 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "bsdconv.h"
#ifndef WIN32
#include <errno.h>
#endif

#define RESET(X) do{	\
	ins->phase[X].index=0;	\
	memcpy(&ins->phase[X].state, ins->phase[X].codec[ins->phase[X].index].z, sizeof(struct state_s));	\
}while(0);

void bsdconv_init(struct bsdconv_instance *ins){
	int i, j;
	struct data_s *data_ptr;

	switch(ins->mode){
		case BSDCONV_BB:
			ins->feed=ins->in_buf;
			ins->feed_len=ins->in_len;
			ins->back=ins->out_buf;
			break;
		case BSDCONV_CB:
			ins->back=ins->out_buf;
			break;
		case BSDCONV_BC:
			ins->feed=ins->in_buf;
			ins->feed_len=ins->in_len;
			break;
		case BSDCONV_BM:
			ins->feed=ins->in_buf;
			ins->feed_len=ins->in_len;
			ins->back=NULL;
			break;
		case BSDCONV_CC:
			break;
		case BSDCONV_CM:
			ins->back=NULL;
			break;
	}

	ins->ierr=0;
	ins->oerr=0;

	ins->from_bak=ins->from_data=ins->feed;
	
	for(i=0;i<=ins->phasen;i++){
		RESET(i)
		ins->phase[i].pend=0;
		while(ins->phase[i].data_head->next){
			data_ptr=ins->phase[i].data_head->next;
			ins->phase[i].data_head->next=ins->phase[i].data_head->next->next;
			free(data_ptr);
		}
		ins->phase[i].data=ins->phase[i].data_tail=ins->phase[i].data_head;
		ins->phase[i].match=NULL;
		for(j=0;j<=ins->phase[i].codecn;j++){
			if(ins->phase[i].codec[j].cbinit)
				ins->phase[i].codec[j].cbinit(&(ins->phase[i].codec[j]),ins->phase[i].codec[j].priv);
		}
	}
}

struct bsdconv_instance *bsdconv_create(const char *conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	char *t, *t2, *p, *cwd;
	int i, j, brk;
	char buf[64], path[PATH_BUF_SIZE];

	i=1;
	for(t=(char *)conversion;*t;t++){
		if(*t==':')++i;
	}
	if(i<2){
		SetLastError(EINVAL);
		return NULL;
	}

	ins->phasen=i-1; //i is real length, but we use i-1 for a convient to use array boundary here
	ins->phase=malloc(sizeof(struct bsdconv_phase) * i);
	char *opipe[i];
	int npipe[i];

	t2=t=strdup(conversion);
	while((*t=toupper(*t))){++t;}
	t=t2;

	if(strcmp("ASCII:FROM_ALIAS:ASCII",conversion)==0 || strcmp("ASCII:INTER_ALIAS:ASCII",conversion)==0 || strcmp("ASCII:TO_ALIAS:ASCII",conversion)==0){
		brk=0;
	}else{
		brk=1;
	}
	for(i=0;i<=ins->phasen;++i){
		opipe[i]=strdup(strsep(&t, ":"));
		if(brk){
			struct bsdconv_instance *alias_ins;
			if(i==0){
				alias_ins=bsdconv_create("ASCII:FROM_ALIAS:ASCII");
			}else if(i==ins->phasen){
				alias_ins=bsdconv_create("ASCII:TO_ALIAS:ASCII");
			}else{
				alias_ins=bsdconv_create("ASCII:INTER_ALIAS:ASCII");
			}
			alias_ins->mode=BSDCONV_CC;
			alias_ins->feed=opipe[i];
			alias_ins->feed_len=strlen(opipe[i]);
			bsdconv_init(alias_ins);
			bsdconv(alias_ins);
			free(opipe[i]);
			opipe[i]=strndup(alias_ins->back,alias_ins->back_len);
			free(alias_ins->back);
			bsdconv_destroy(alias_ins);
		}
	}

	for(i=0;i<=ins->phasen;++i){
		if(*opipe[i]){
			npipe[i]=1;
			for(t=(char *)opipe[i];*t;t++){
				if(*t==','){
					npipe[i]++;
				}
			}
		}else{
			npipe[i]=0;
		}
		ins->phase[i].codecn=npipe[i];
		if(npipe[i]==0){
			SetLastError(EINVAL);
			return NULL;
		}
	}
	free(t2);

	cwd=getwd(NULL);

	if((p=getenv("BSDCONV_PATH"))){
		chdir(p);
	}else{
		chdir(PREFIX);
	}
	chdir("share/bsdconv");

	for(i=0;i<=ins->phasen;++i){
		ins->phase[i].codec=malloc(npipe[i] * sizeof(struct bsdconv_codec_t));
		if(i==0){
			chdir("from");
		}else if(i==ins->phasen){
			chdir("to");
		}else{
			chdir("inter");
		}
		brk=0;
		t=opipe[i];
		for(j=0;j<ins->phase[i].codecn;++j){
			ins->phase[i].codec[j].desc=strdup(strsep(&t, ","));
			strcpy(buf, ins->phase[i].codec[j].desc);
			REALPATH(buf, path);
			if(!loadcodec(&ins->phase[i].codec[j], path, 0)){
				return NULL;
			}
		}
		chdir("..");

		ins->phase[i].codecn--;
		ins->phase[i].data_head=malloc(sizeof(struct data_s));
		ins->phase[i].data_head->next=NULL;
		for(j=0;j<=ins->phase[i].codecn;j++){
			if(ins->phase[i].codec[j].cbcreate){
				ins->phase[i].codec[j].priv=ins->phase[i].codec[j].cbcreate();
			}
		}
		free(opipe[i]);
	}

	chdir(cwd);
	free(cwd);

	return ins;
}

void bsdconv_destroy(struct bsdconv_instance *ins){
	int i,j;
	struct data_s *data_ptr;

	for(i=0;i<=ins->phasen;i++){
		for(j=0;j<=ins->phase[i].codecn;j++){
			free(ins->phase[i].codec[j].desc);
			if(ins->phase[i].codec[j].cbdestroy){
				ins->phase[i].codec[j].cbdestroy(ins->phase[i].codec[j].priv);
			}
			unloadcodec(&ins->phase[i].codec[j]);
		}
		free(ins->phase[i].codec);
		while(ins->phase[i].data_head){
			data_ptr=ins->phase[i].data_head;
			ins->phase[i].data_head=ins->phase[i].data_head->next;
			free(data_ptr);
		}
	}
	free(ins->phase);
	free(ins);
}

#define check_leftovers() do{	\
	for(ins->phase_index=ins->phasen-1;ins->phase_index>=0;--ins->phase_index){	\
		if(ins->phase[ins->phase_index].data->next){	\
			if(ins->phase_index==ins->phasen-1){	\
				goto phase_to;	\
			}else{	\
				ins->phase_index++;	\
				goto phase_inter;	\
			}	\
		}	\
	}	\
	if(ins->from_data < ins->feed+ins->feed_len) goto phase_from;	\
}while(0);

#define check_pending() do{	\
	for(ins->phase_index=0;ins->phase_index<=ins->phasen;++ins->phase_index){	\
		if(ins->phase[ins->phase_index].pend){	\
			if(ins->phase_index==0){	\
				goto pass_to_inter;	\
			}else if(ins->phase_index==ins->phasen){	\
				goto pass_to_out;	\
			}else{	\
				goto pass_to_to;	\
			}	\
		}	\
	}	\
}while(0);

int bsdconv(struct bsdconv_instance *ins){
	uintptr_t i;
	struct data_s *data_ptr;
	unsigned char *ptr;

	switch(ins->mode){
		case BSDCONV_BB:
			ins->back_len=0;
			if(ins->phase[ins->phasen].data_head->next) goto bb_out;
			check_leftovers();
			break;
		case BSDCONV_BC:
			check_leftovers();
			break;
		case BSDCONV_CB:
			ins->back_len=0;
			if(ins->phase[ins->phasen].data_head->next) goto cb_out;
			break;
		case BSDCONV_CC:
			break;
		case BSDCONV_BM:
			if(ins->back){
				ptr=ins->back;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					free(data_ptr);
				}
				return 0;
			}else{
				for(ins->phase_index=ins->phasen-1;ins->phase_index>=0;--ins->phase_index){
					if(ins->phase[ins->phase_index].data->next){
						if(ins->phase_index==ins->phasen-1){
							goto phase_to;
						}else{
							ins->phase_index++;
							goto phase_inter;
						}
					}
				}
			}
			break;
		case BSDCONV_CM:
			if(ins->back){
				ptr=ins->back;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					free(data_ptr);
				}
				return 0;
			}
			break;
		}

	//from
	phase_from:
	while(ins->from_data < ins->feed+ins->feed_len){
		memcpy(&ins->phase[0].state, ins->phase[0].codec[ins->phase[0].index].z + (uintptr_t)ins->phase[0].state.sub[*ins->from_data], sizeof(struct state_s));
		from_x:
		switch(ins->phase[0].state.status){
			case DEADEND:
				pass_to_inter:
				ins->phase[0].pend=0;
				if(ins->phase[0].match){
					LISTCPY(ins->phase[0].data_tail, ins->phase[0].match, ins->phase[0].codec[ins->phase[0].index].data_z);
					ins->phase[0].match=NULL;
					RESET(0);

					ins->from_data=ins->from_bak;
					ins->phase_index=1;
					goto phase_inter;
				}else if(ins->phase[0].index < ins->phase[0].codecn){
					ins->phase[0].index++;
					memcpy(&ins->phase[0].state, ins->phase[0].codec[ins->phase[0].index].z, sizeof(struct state_s));

					ins->from_data=ins->from_bak;
					continue;
				}else{
					ins->ierr++;

					RESET(0);

					ins->from_data=ins->from_bak;
					++ins->from_data;
					ins->from_bak=ins->from_data;
					continue;
				}
				break;
			case MATCH:
				++ins->from_data;
				ins->from_bak=ins->from_data;
				LISTCPY(ins->phase[0].data_tail, ins->phase[0].state.data, ins->phase[0].codec[ins->phase[0].index].data_z);
				ins->phase[0].pend=0;
				ins->phase[0].match=NULL;
				RESET(0);
				ins->phase_index=1;
				goto phase_inter;
			case SUBMATCH:
				ins->phase[0].match=ins->phase[0].state.data;
				++ins->from_data;
				ins->from_bak=ins->from_data;
				ins->phase[0].pend=1;
				break;
			case SUBROUTINE:
				ins->phase[0].codec[ins->phase[0].index].callback(ins);
				goto from_x;
			case NEXTPHASE:
				ins->phase[0].pend=0;

				++ins->from_data;
				ins->from_bak=ins->from_data;
				ins->phase_index=1;

				RESET(0);
				goto phase_inter;
			case CONTINUE:
				ins->phase[0].pend=1;
				++ins->from_data;
				break;
		}
	}
	ins->phase_index=1;

	//inter
	phase_inter:
	if(ins->phase_index==ins->phasen){
		goto phase_to;
	}
	while(ins->phase[ins->phase_index-1].data->next){
		ins->phase[ins->phase_index-1].data=ins->phase[ins->phase_index-1].data->next;
		for(i=0;i<ins->phase[ins->phase_index-1].data->len;i++){
			memcpy(&ins->phase[ins->phase_index].state, ins->phase[ins->phase_index].codec[ins->phase[ins->phase_index].index].z + (uintptr_t)ins->phase[ins->phase_index].state.sub[*(ins->phase[ins->phase_index-1].data->data+i)], sizeof(struct state_s));
			switch(ins->phase[ins->phase_index].state.status){
				case DEADEND:
					goto pass_to_to;
					break;
				case SUBROUTINE:
					goto to_callback;
					break;
			}
		}
		inter_x:
		switch(ins->phase[ins->phase_index].state.status){
			case DEADEND:
				pass_to_to:
				ins->phase[ins->phase_index].pend=0;
				if(ins->phase[ins->phase_index].match){
					LISTCPY(ins->phase[ins->phase_index].data_tail, ins->phase[ins->phase_index].match, ins->phase[ins->phase_index].codec[ins->phase[ins->phase_index].index].data_z);

					LISTFREE(ins->phase[ins->phase_index-1].data_head,ins->phase[ins->phase_index].bak,ins->phase[ins->phase_index-1].data_tail);
					ins->phase[ins->phase_index-1].data=ins->phase[ins->phase_index-1].data_head;

					ins->phase[ins->phase_index].match=0;
					RESET(ins->phase_index);
					goto phase_inter;
				}else if(ins->phase[ins->phase_index].index < ins->phase[ins->phase_index].codecn){
					ins->phase[ins->phase_index].index++;
					memcpy(&ins->phase[ins->phase_index].state, ins->phase[ins->phase_index].codec[ins->phase[ins->phase_index].index].z, sizeof(struct state_s));
					ins->phase[ins->phase_index-1].data=ins->phase[ins->phase_index-1].data_head;
					continue;
				}else{
					data_ptr=ins->phase[ins->phase_index-1].data_head->next;
					ins->phase[ins->phase_index-1].data_head->next=ins->phase[ins->phase_index-1].data_head->next->next;
					ins->phase[ins->phase_index-1].data=ins->phase[ins->phase_index-1].data_head;
					data_ptr->next=NULL;
					ins->phase[ins->phase_index].data_tail->next=data_ptr;
					ins->phase[ins->phase_index].data_tail=data_ptr;
					if(ins->phase[ins->phase_index-1].data_tail==data_ptr){
						ins->phase[ins->phase_index-1].data_tail=ins->phase[ins->phase_index-1].data_head;
					}
					ins->phase[ins->phase_index-1].data=ins->phase[ins->phase_index-1].data_head;

					RESET(ins->phase_index);

					++ins->phase_index;
					goto phase_inter;
				}
				break;
			case MATCH:
				ins->phase[ins->phase_index].match=0;
				ins->phase[ins->phase_index].pend=0;

				LISTCPY(ins->phase[ins->phase_index].data_tail, ins->phase[ins->phase_index].state.data, ins->phase[ins->phase_index].codec[ins->phase[ins->phase_index].index].data_z);

				ins->phase[ins->phase_index].bak=ins->phase[ins->phase_index-1].data->next;
				LISTFREE(ins->phase[ins->phase_index-1].data_head,ins->phase[ins->phase_index].bak,ins->phase[ins->phase_index-1].data_tail);
				ins->phase[ins->phase_index-1].data=ins->phase[ins->phase_index-1].data_head;

				RESET(ins->phase_index);

				++ins->phase_index;
				goto phase_inter;
			case SUBMATCH:
				ins->phase[ins->phase_index].match=ins->phase[ins->phase_index].state.data;
				ins->phase[ins->phase_index].bak=ins->phase[ins->phase_index-1].data->next;
				ins->phase[ins->phase_index].pend=1;
				break;
			case NEXTPHASE:
				ins->phase[ins->phase_index].match=0;
				ins->phase[ins->phase_index].pend=0;
				++ins->phase_index;

				RESET(ins->phase_index);
				goto phase_inter;
			case SUBROUTINE:
				//inter_callback:
				ins->phase[ins->phase_index].codec[ins->phase[ins->phase_index].index].callback(ins);
				goto inter_x;
			case CONTINUE:
				ins->phase[ins->phase_index].pend=1;
				break;
		}
		memcpy(&ins->phase[ins->phase_index].state, ins->phase[ins->phase_index].codec[ins->phase[ins->phase_index].index].z + (uintptr_t)ins->phase[ins->phase_index].state.sub[256], sizeof(struct state_s));
		if(ins->phase[ins->phase_index].state.status==DEADEND){ goto pass_to_to;}
	}
	++ins->phase_index;
	goto phase_inter;

	//to
	phase_to:
	while(ins->phase[ins->phasen-1].data->next){
		ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data->next;
		for(i=0;i<ins->phase[ins->phasen-1].data->len;i++){
			memcpy(&ins->phase[ins->phasen].state, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].z + (uintptr_t)ins->phase[ins->phasen].state.sub[*(ins->phase[ins->phasen-1].data->data+i)], sizeof(struct state_s));
			switch(ins->phase[ins->phasen].state.status){
				case DEADEND:
					goto pass_to_out;
					break;
				case SUBROUTINE:
					goto to_callback;
					break;
			}
		}
		to_x:
		switch(ins->phase[ins->phasen].state.status){
			case DEADEND:
				pass_to_out:
				ins->phase[ins->phasen].pend=0;
				if(ins->phase[ins->phasen].match){
					LISTCPY(ins->phase[ins->phasen].data_tail, ins->phase[ins->phasen].match, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].data_z);

					LISTFREE(ins->phase[ins->phasen-1].data_head,ins->phase[ins->phasen].bak,ins->phase[ins->phasen-1].data_tail);
					ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;

					ins->phase[ins->phasen].match=0;
					RESET(ins->phasen);
					goto phase_out;
				}else if(ins->phase[ins->phasen].index < ins->phase[ins->phasen].codecn){
					ins->phase[ins->phasen].index++;
					memcpy(&ins->phase[ins->phasen].state, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].z, sizeof(struct state_s));
					ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;
					continue;
				}else{
					ins->oerr++;

					RESET(ins->phasen);

					ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
					LISTFREE(ins->phase[ins->phasen-1].data_head,ins->phase[ins->phasen].bak,ins->phase[ins->phasen-1].data_tail);
					ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;

					continue;
				}
				break;
			case MATCH:
				ins->phase[ins->phasen].match=0;
				ins->phase[ins->phasen].pend=0;

				LISTCPY(ins->phase[ins->phasen].data_tail, ins->phase[ins->phasen].state.data, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].data_z);

				ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
				LISTFREE(ins->phase[ins->phasen-1].data_head, ins->phase[ins->phasen].bak,ins->phase[ins->phasen-1].data_tail);
				ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;

				RESET(ins->phasen);
				goto phase_out;
			case SUBMATCH:
				ins->phase[ins->phasen].match=ins->phase[ins->phasen].state.data;
				ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
				ins->phase[ins->phasen].pend=1;
				break;
			case SUBROUTINE:
				to_callback:
				ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].callback(ins);
				goto to_x;
			case NEXTPHASE:
				ins->phase[ins->phasen].match=0;
				ins->phase[ins->phasen].pend=0;

				ins->phase[ins->phasen].bak=ins->phase[ins->phasen-1].data->next;
				LISTFREE(ins->phase[ins->phasen-1].data_head,ins->phase[ins->phasen].bak,ins->phase[ins->phasen-1].data_tail);
				ins->phase[ins->phasen-1].data=ins->phase[ins->phasen-1].data_head;

				RESET(ins->phasen);

				goto phase_out;
			case CONTINUE:
				ins->phase[ins->phasen].pend=1;
				break;
		}
		memcpy(&ins->phase[ins->phasen].state, ins->phase[ins->phasen].codec[ins->phase[ins->phasen].index].z + (uintptr_t)ins->phase[ins->phasen].state.sub[256], sizeof(struct state_s));
		if(ins->phase[ins->phasen].state.status==DEADEND){ goto pass_to_out;}
	}

	phase_out:
	switch(ins->mode){
		case BSDCONV_BB:
			bb_out:
			while(ins->phase[ins->phasen].data_head->next){
				i=ins->back_len + ins->phase[ins->phasen].data_head->next->len;
				if(i > ins->out_len){
					goto bb_hibernate;
				}else{
					memcpy(ins->back + ins->back_len, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
					ins->back_len=i;
				}
				if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
					ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
				}
				data_ptr=ins->phase[ins->phasen].data_head->next;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}
			check_leftovers();
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				check_pending();
				return 0;
			}else{
			bb_hibernate:
				ins->from_data-=(ins->from_bak - ins->in_buf);
				i=(ins->feed+ins->feed_len)-ins->from_bak;
				memmove(ins->in_buf, ins->from_bak, i);
				ins->feed=ins->in_buf+i;
				ins->feed_len=ins->in_len - i;
				ins->from_bak=ins->in_buf;
				return 1;
			}
			break;
		case BSDCONV_BC:
			check_leftovers();
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				check_pending();
				i=0;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ptr=ins->out_buf=ins->back=malloc(i);
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					free(data_ptr);
				}
				return 0;
			}else{
			//bc_hibernate:
				ins->from_data-=(ins->from_bak - ins->in_buf);
				i=(ins->feed+ins->feed_len)-ins->from_bak;
				memmove(ins->in_buf, ins->from_bak, i);
				ins->feed=ins->in_buf+i;
				ins->feed_len=ins->in_len - i;
				ins->from_bak=ins->in_buf;
				return 1;
			}
			break;
		case BSDCONV_CB:
			cb_out:
			while(ins->phase[ins->phasen].data_head->next){
				i=ins->back_len + ins->phase[ins->phasen].data_head->next->len;
				if(i > ins->out_len){
					return 1;
				}else{
					memcpy(ins->back + ins->back_len, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
					ins->back_len=i;
				}
				if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
					ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
				}
				data_ptr=ins->phase[ins->phasen].data_head->next;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}

			
			check_leftovers();

			check_pending();
			return 0;
			break;
		case BSDCONV_CC:
			check_leftovers();

			check_pending();

			i=0;
			data_ptr=ins->phase[ins->phasen].data_head->next;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->back_len=ins->out_len=i;
			ptr=ins->back=ins->out_buf=ins->back=malloc(i);
			data_ptr=ins->phase[ins->phasen].data_head;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				memcpy(ptr, data_ptr->data, data_ptr->len);
				ptr+=data_ptr->len;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				free(data_ptr->data);
				free(data_ptr);
			}
			return 0;
			break;
		case BSDCONV_BM:
			check_leftovers();
			if(ins->feed+ins->feed_len<ins->in_buf+ins->in_len){
				check_pending();
				i=0;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ins->back_len=i;
				return 0;
			}else{
				ins->from_data-=(ins->from_bak - ins->in_buf);
				i=(ins->feed+ins->feed_len)-ins->from_bak;
				memmove(ins->in_buf, ins->from_bak, i);
				ins->feed=ins->in_buf+i;
				ins->feed_len=ins->in_len - i;
				ins->from_bak=ins->in_buf;
				return 1;
			}
			break;
		case BSDCONV_CM:
			check_leftovers();
			check_pending();
			i=0;
			data_ptr=ins->phase[ins->phasen].data_head->next;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->back_len=i;
			return 0;
			break;
	}
	return 1;
}

char * bsdconv_error(void){
	switch(GetLastError()){
		case EOPNOTSUPP:
				return strdup("Unsupported charset/encoding");
		case ENOMEM:
				return strdup("Mmap failed");
		case EINVAL:
				return strdup("Conversion syntax error");
		default:
				return strdup("Unknown error");;
	}
}
