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
	memcpy(&ins->phase[X].state, ins->phase[X].codec[ins->phase[X].index].z, sizeof(struct state_st));	\
}while(0);

void bsdconv_init(struct bsdconv_instance *ins){
	int i, j;
	struct data_rt *data_ptr;

	ins->input.data=NULL;
	ins->input.len=0;
	ins->output.data=NULL;
	ins->output.len=0;
	ins->output_mode=BSDCONV_HOLD;

	ins->ierr=0;
	ins->oerr=0;
	
	for(i=0;i<=ins->phasen;i++){
		ins->phase[i].pend=0;
		while(ins->phase[i].data_head->next){
			data_ptr=ins->phase[i].data_head->next;
			ins->phase[i].data_head->next=ins->phase[i].data_head->next->next;
			if(data_ptr->setmefree)
				free(data_ptr->data);
			free(data_ptr);
		}
		ins->phase[i].data_tail=ins->phase[i].data_head;
		ins->phase[i].data_head->len=0;
		ins->phase[i].match=NULL;
		if(i>0){
			ins->phase[i].data=ins->phase[i-1].data_head;
			RESET(i)
			for(j=0;j<=ins->phase[i].codecn;j++){
				if(ins->phase[i].codec[j].cbinit)
					ins->phase[i].codec[j].cbinit(&(ins->phase[i].codec[j]),ins->phase[i].codec[j].priv);
			}
		}
	}
}

struct bsdconv_instance *bsdconv_create(const char *conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	char *t, *t1, *t2, *p, *cwd;
	int i, j, brk;
	char buf[64], path[PATH_BUF_SIZE];

	i=1;
	for(t=(char *)conversion;*t;t++){
		if(*t==':' || *t=='|')++i;
	}
	if(i<2){
		SetLastError(EINVAL);
		return NULL;
	}
	ins->phasen=i;
	ins->phase=malloc(sizeof(struct bsdconv_phase) * (i+1));
	char *opipe[i+1];

	t2=t1=t=strdup(conversion);
	while((*t=toupper(*t))){++t;}
	t=t1;

	i=1;
	while((t1=strsep(&t, "|")) != NULL){
		brk=1;
		while((opipe[i]=strsep(&t1, ":"))!=NULL){
			opipe[i]=strdup(opipe[i]);
			if(brk){
				ins->phase[i].type=FROM;
				ins->phase[i-1].type=TO;
				brk=0;
			}else{
				ins->phase[i].type=INTER;
			}
			i+=1;
		}
	}
	ins->phase[0].type=INPUT;
	ins->phase[i-1].type=TO;

	if(strcmp("ASCII:FROM_ALIAS:ASCII",conversion)==0 || strcmp("ASCII:INTER_ALIAS:ASCII",conversion)==0 || strcmp("ASCII:TO_ALIAS:ASCII",conversion)==0){
		brk=1;
	}else{
		brk=0;
	}
	for(i=1;i<=ins->phasen;++i){
		if(brk==0){
			struct bsdconv_instance *alias_ins;
			switch(ins->phase[i].type){
				case FROM:
					alias_ins=bsdconv_create("ASCII:FROM_ALIAS:ASCII");
					break;
				case INTER:
					alias_ins=bsdconv_create("ASCII:INTER_ALIAS:ASCII");
					break;
				case TO:
					alias_ins=bsdconv_create("ASCII:TO_ALIAS:ASCII");
					break;
				default:
					alias_ins=NULL;
					return NULL;
			}
			if(alias_ins==NULL){
				SetLastError(EDOOFUS);
				return NULL;
			}
			bsdconv_init(alias_ins);
			alias_ins->output_mode=BSDCONV_AUTOMALLOC;
			alias_ins->input.data=opipe[i];
			alias_ins->input.len=strlen(opipe[i]);
			alias_ins->input.setmefree=0;
			alias_ins->flush=1;
			bsdconv(alias_ins);
			opipe[i]=alias_ins->output.data;
			bsdconv_destroy(alias_ins);
		}
		if(*opipe[i]){
			ins->phase[i].codecn=1;
			for(t=(char *)opipe[i];*t;t++){
				if(*t==','){
					ins->phase[i].codecn+=1;
				}
			}
		}else{
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
	ins->phase[0].data_tail=ins->phase[0].data_head=malloc(sizeof(struct data_st));
	ins->phase[0].data_head->next=NULL;
	for(i=1;i<=ins->phasen;++i){
		ins->phase[i].codec=malloc(ins->phase[i].codecn * sizeof(struct bsdconv_codec_t));
		switch(ins->phase[i].type){
			case FROM:
				chdir("from");
				break;
			case INTER:
				chdir("inter");
				break;
			case TO:
				chdir("to");
				break;
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
		ins->phase[i].data_head=malloc(sizeof(struct data_st));
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
	struct data_rt *data_ptr;

	for(i=1;i<=ins->phasen;i++){
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

void bsdconv(struct bsdconv_instance *ins){
	uintptr_t i=0;
	struct data_rt *data_ptr;
	char *ptr;
	struct bsdconv_phase *this_phase;
	struct bsdconv_phase *prev_phase;

	if(ins->input.len){
		ins->phase[0].data_tail->next=malloc(sizeof(struct data_rt));
		ins->phase[0].data_tail=ins->phase[0].data_tail->next;
		*(ins->phase[0].data_tail)=ins->input;
		ins->input.data=NULL;
		ins->input.setmefree=0;
	}

	ins->phase_index=1;

	phase_begin:
	if(ins->phase_index>0 && ins->phase_index<=ins->phasen){
		this_phase=&ins->phase[ins->phase_index];
		prev_phase=&ins->phase[ins->phase_index-1];
		switch(this_phase->type){
			case FROM:
				while(this_phase->data->next){
					if(this_phase->data == prev_phase->data_head) i=this_phase->data_head->len;
					else i=0;
					this_phase->data=this_phase->data->next;
					while(i<this_phase->data->len){
						memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[UCP(this_phase->data->data)[i]], sizeof(struct state_st));
						from_x:
						switch(this_phase->state.status){
							case DEADEND:
								from_deadend:
								this_phase->pend=0;
								if(this_phase->match){
									LISTCPY(this_phase->data_tail, this_phase->match, this_phase->codec[this_phase->index].data_z);

									LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
									this_phase->data=prev_phase->data_head;
									i=this_phase->data_head->len;

									this_phase->match=0;
									RESET(ins->phase_index);
									goto phase_begin;
								}else if(this_phase->index < this_phase->codecn){
									this_phase->index++;
									memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
									this_phase->data=prev_phase->data_head;
									i=this_phase->data_head->len;
									continue;
								}else{
									ins->ierr++;

									RESET(ins->phase_index);

									this_phase->bak=this_phase->data;
									LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
									this_phase->bak=this_phase->data=prev_phase->data_head;
									this_phase->data_head->len=i+1;
									continue;
								}
								break;
							case MATCH:
								this_phase->match=0;
								this_phase->pend=0;

								LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

								this_phase->bak=this_phase->data;
								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->data=prev_phase->data_head;
								this_phase->data_head->len=i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;

							case SUBMATCH:
								this_phase->match=this_phase->state.data;

								this_phase->bak=this_phase->data;
								this_phase->data_head->len=i+1;

								this_phase->pend=1;
								break;
							case SUBROUTINE:
								this_phase->i=i;
								this_phase->codec[this_phase->index].callback(ins);
								goto from_x;
							case NEXTPHASE:
								this_phase->match=0;
								this_phase->pend=0;

								this_phase->bak=this_phase->data;
								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->data=prev_phase->data_head;
								this_phase->data_head->len=i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;
							case CONTINUE:
								this_phase->pend=1;
								break;
						}
						i+=1;
					}
				}
			break;

		case INTER:
			while(this_phase->data->next){
				this_phase->data=this_phase->data->next;
				this_phase->state.status=DUMMY;
				for(i=0;i<this_phase->data->len;i++){
					memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[UCP(this_phase->data->data)[i]], sizeof(struct state_st));
					switch(this_phase->state.status){
						case DEADEND:
							goto inter_deadend;
							break;
						case SUBROUTINE:
							goto inter_callback;
							break;
					}
				}
				inter_x:
				switch(this_phase->state.status){
					case DUMMY:
						continue;
					case DEADEND:
						inter_deadend:
						this_phase->pend=0;
						if(this_phase->match){
							LISTCPY(this_phase->data_tail, this_phase->match, this_phase->codec[this_phase->index].data_z);

							LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
							this_phase->data=prev_phase->data_head;

							this_phase->match=0;
							RESET(ins->phase_index);
							goto phase_begin;
						}else if(this_phase->index < this_phase->codecn){
							this_phase->index++;
							memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
							this_phase->data=prev_phase->data_head;
							continue;
						}else{
							data_ptr=prev_phase->data_head->next;
							prev_phase->data_head->next=prev_phase->data_head->next->next;
							this_phase->data=prev_phase->data_head;
							data_ptr->next=NULL;
							this_phase->data_tail->next=data_ptr;
							this_phase->data_tail=data_ptr;
							if(prev_phase->data_tail==data_ptr){
								prev_phase->data_tail=prev_phase->data_head;
							}
							this_phase->data=prev_phase->data_head;

							RESET(ins->phase_index);

							ins->phase_index+=1;
							goto phase_begin;
						}
						break;
					case MATCH:
						this_phase->match=0;
						this_phase->pend=0;

						LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

						this_phase->bak=this_phase->data->next;
						LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
						this_phase->data=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case SUBMATCH:
						this_phase->match=this_phase->state.data;

						if(this_phase->data->next){
							this_phase->bak=this_phase->data->next;
						}else{
							this_phase->bak=prev_phase->data_tail->next=malloc(sizeof(struct data_st));
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->setmefree=0;
						}

						this_phase->pend=1;
						break;
					case SUBROUTINE:
						inter_callback:
						this_phase->codec[this_phase->index].callback(ins);
						goto inter_x;
					case NEXTPHASE:
						this_phase->match=0;
						this_phase->pend=0;

						this_phase->bak=this_phase->data->next;
						LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
						this_phase->data=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case CONTINUE:
						this_phase->pend=1;
						break;
				}
				memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[256], sizeof(struct state_st));
				if(this_phase->state.status==DEADEND){ goto inter_deadend;}
			}
			break;

		case TO:
			while(this_phase->data->next){
				this_phase->data=this_phase->data->next;
				this_phase->state.status=DUMMY;
				for(i=0;i<this_phase->data->len;i++){
					memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[UCP(this_phase->data->data)[i]], sizeof(struct state_st));
					switch(this_phase->state.status){
						case DEADEND:
							goto to_deadend;
							break;
						case SUBROUTINE:
							goto to_callback;
							break;
					}
				}
				to_x:
				switch(this_phase->state.status){
					case DUMMY:
						continue;
					case DEADEND:
						to_deadend:
						this_phase->pend=0;
						if(this_phase->match){
							LISTCPY(this_phase->data_tail, this_phase->match, this_phase->codec[this_phase->index].data_z);

							LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
							this_phase->data=prev_phase->data_head;

							this_phase->match=0;
							RESET(ins->phase_index);
							ins->phase_index+=1;
							goto phase_begin;
						}else if(this_phase->index < this_phase->codecn){
							this_phase->index++;
							memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
							this_phase->data=prev_phase->data_head;
							continue;
						}else{
							ins->oerr++;

							RESET(ins->phase_index);

							this_phase->bak=this_phase->data->next;
							LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
							this_phase->bak=this_phase->data=prev_phase->data_head;

							continue;
						}
						break;
					case MATCH:
						this_phase->match=0;
						this_phase->pend=0;

						LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

						this_phase->bak=this_phase->data->next;
						LISTFREE(prev_phase->data_head, this_phase->bak,prev_phase->data_tail);
						this_phase->data=prev_phase->data_head;

						RESET(ins->phase_index);
						ins->phase_index+=1;
						goto phase_begin;
					case SUBMATCH:
						this_phase->match=this_phase->state.data;
						if(this_phase->data->next){
							this_phase->bak=this_phase->data->next;
						}else{
							this_phase->bak=prev_phase->data_tail->next=malloc(sizeof(struct data_st));
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->setmefree=0;
						}

						this_phase->pend=1;
						break;
					case SUBROUTINE:
						to_callback:
						this_phase->codec[this_phase->index].callback(ins);
						goto to_x;
					case NEXTPHASE:
						this_phase->match=0;
						this_phase->pend=0;

						this_phase->bak=this_phase->data->next;
						LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
						this_phase->data=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case CONTINUE:
						this_phase->pend=1;
						break;
				}
				memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[256], sizeof(struct state_st));
				if(this_phase->state.status==DEADEND){ goto to_deadend;}
			}
			break;
		}
		ins->phase_index+=1;
	}

	//check back (phase-loop)
	for(ins->phase_index=ins->phasen;ins->phase_index>0;ins->phase_index-=1){
		if(ins->phase[ins->phase_index].data->next){
			goto phase_begin;
		}
	}

	//flush
	if(ins->flush){
		for(ins->phase_index=1;ins->phase_index<=ins->phasen;++(ins->phase_index)){
			if(ins->phase[ins->phase_index].pend){
				this_phase=&ins->phase[ins->phase_index];
				prev_phase=&ins->phase[ins->phase_index-1];
				switch(this_phase->type){
					case FROM:	goto from_deadend;
					case INTER:	goto inter_deadend;
					case TO:	goto to_deadend;
				}
			}
		}
	}

	//output
	switch(ins->output_mode){
		case BSDCONV_HOLD:
			ins->output.len=0;
			break;
		case BSDCONV_AUTOMALLOC:
			i=4;
			data_ptr=ins->phase[ins->phasen].data_head;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			if(i>4){
				ins->output.setmefree=1;
				ptr=ins->output.data=malloc(i);
				ins->output.len=i-4;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(ins->phase[ins->phasen].data_head->next){
					data_ptr=ins->phase[ins->phasen].data_head->next;
					memcpy(ptr, data_ptr->data, data_ptr->len);
					ptr+=data_ptr->len;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					if(data_ptr->setmefree)
						free(data_ptr->data);
					free(data_ptr);
				}
				ptr[0]=ptr[1]=ptr[2]=ptr[3]=0;
			}
			break;
		case BSDCONV_PREMALLOC:
			ins->output.setmefree=0;
			i=0;
			while(ins->phase[ins->phasen].data_head->next && ins->phase[ins->phasen].data_head->next->len<=ins->output.len-i){
				memcpy(ins->output.data+i, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
				i+=ins->phase[ins->phasen].data_head->next->len;
				if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
					ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
				}
				data_ptr=ins->phase[ins->phasen].data_head->next;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				if(data_ptr->setmefree)
					free(data_ptr->data);
				free(data_ptr);
			}
			ins->output.len=i;
			break;
		case BSDCONV_FILE:
			
			break;
		case BSDCONV_FD:

			break;
	}
	return;
}

char * bsdconv_error(void){
	switch(GetLastError()){
		case EDOOFUS:
				return strdup("Unexpected condition");
		case EOPNOTSUPP:
				return strdup("Unsupported charset/encoding");
		case ENOMEM:
				return strdup("Mmap failed");
		case EINVAL:
				return strdup("Conversion syntax error");
		default:
				return strdup("Unknown error");
	}
}
