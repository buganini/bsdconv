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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "bsdconv.h"
#ifdef WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#ifndef MAP_PREFAULT_READ
#define MAP_PREFAULT_READ 0
#endif

int _loadcodec(struct bsdconv_codec_t *cd, char *path){
#ifdef WIN32
	if ((cd->fd=CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))==INVALID_HANDLE_VALUE){
		SetLastError(EOPNOTSUPP);
		return 0;
	}
	cd->md=CreateFileMapping(cd->fd, NULL, PAGE_READONLY, 0,0, NULL);
	if(!cd->md){
		CloseHandle(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
	cd->data_z=cd->z=MapViewOfFile(cd->md, FILE_MAP_READ, 0,0,0);
	if(!cd->z){
		CloseHandle(cd->md);
		CloseHandle(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
#else
	struct stat stat;
	if((cd->fd=open(path, O_RDONLY))==-1){
		SetLastError(EOPNOTSUPP);
		return 0;
	}
	fstat(cd->fd, &stat);
	cd->maplen=stat.st_size;
	if((cd->data_z=cd->z=mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE | MAP_PREFAULT_READ, cd->fd, 0))==MAP_FAILED){
		close(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
#endif

	cd->dl=NULL;
	cd->cbcreate=NULL;
	cd->cbinit=NULL;
	cd->callback=NULL;
	cd->cbdestroy=NULL;
	strcat(path, "." SHLIBEXT);

#ifdef WIN32
	if((cd->dl=LoadLibrary(path))){
		cd->callback=(void *)GetProcAddress(cd->dl,"callback");
		cd->cbcreate=(void *)GetProcAddress(cd->dl,"cbcreate");
		cd->cbinit=(void *)GetProcAddress(cd->dl,"cbinit");
		cd->cbdestroy=(void *)GetProcAddress(cd->dl,"cbdestroy");
	}
#else
	if((cd->dl=dlopen(path, RTLD_LAZY))){
		cd->callback=dlsym(cd->dl,"callback");
		cd->cbcreate=dlsym(cd->dl,"cbcreate");
		cd->cbinit=dlsym(cd->dl,"cbinit");
		cd->cbdestroy=dlsym(cd->dl,"cbdestroy");
		if(cd->cbcreate && cd->cbdestroy==NULL){
			fprintf(stderr,"Possible memory leak in %s\n", path);
		}
	}
#endif
	return 1;
}

int loadcodec(struct bsdconv_codec_t *cd, int type, const char *codec){
	char *cwd;
	char *c;
	char buf[PATH_BUF_SIZE];
	cd->desc=strdup(codec);
	for(c=cd->desc;*c;++c){
		if(*c>='a' && *c<='z'){
			*c=*c-'a'+'A';
		}
	}
	cwd=getwd(NULL);
	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(PREFIX);
	}
	chdir("share/bsdconv");
	switch(type){
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
	REALPATH(cd->desc, buf);
	if(!_loadcodec(cd, buf)){
		struct bsdconv_instance *alias_ins;
		switch(type){
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
				SetLastError(EDOOFUS);
				chdir(cwd);
				free(cwd);
				return 0;
		}
		if(alias_ins==NULL){
			SetLastError(EDOOFUS);
			chdir(cwd);
			free(cwd);
			return 0;
		}
		bsdconv_init(alias_ins);
		alias_ins->output_mode=BSDCONV_AUTOMALLOC;
		alias_ins->output.len=1;
		alias_ins->input.data=cd->desc;
		alias_ins->input.len=strlen(cd->desc);
		alias_ins->input.flags|=F_FREE;
		alias_ins->flush=1;
		bsdconv(alias_ins);
		cd->desc=alias_ins->output.data;
		cd->desc[alias_ins->output.len]=0;
		bsdconv_destroy(alias_ins);
		REALPATH(cd->desc, buf);
		if(!_loadcodec(cd, buf)){
			chdir(cwd);
			free(cwd);
			return 0;
		}
	}
	chdir(cwd);
	free(cwd);
	return 1;
}

void unloadcodec(struct bsdconv_codec_t *cd){
#ifdef WIN32
	if(cd->dl){
		FreeLibrary(cd->dl);
	}
	UnmapViewOfFile(cd->z);
	CloseHandle(cd->md);
	CloseHandle(cd->fd);
#else
	if(cd->dl){
		dlclose(cd->dl);
	}
	munmap(cd->z, cd->maplen);
	close(cd->fd);
#endif
}

void bsdconv_init(struct bsdconv_instance *ins){
	int i, j;
	struct data_rt *data_ptr;

	ins->flush=0;
	ins->input.data=NULL;
	ins->input.len=0;
	ins->output.data=NULL;
	ins->output.len=0;
	ins->output_mode=BSDCONV_HOLD;

	ins->ierr=0;
	ins->oerr=0;
	ins->score=0;

	for(i=0;i<=ins->phasen;++i){
		ins->phase[i].pend=0;
		while(ins->phase[i].data_head->next){
			data_ptr=ins->phase[i].data_head->next;
			ins->phase[i].data_head->next=ins->phase[i].data_head->next->next;
			if(data_ptr->flags & F_FREE)
				free(data_ptr->data);
			free(data_ptr);
		}
		ins->phase[i].data_tail=ins->phase[i].data_head;
		ins->phase[i].data_head->len=0;
		ins->phase[i].match=NULL;
		if(i>0){
			ins->phase[i].data=ins->phase[i-1].data_head;
			RESET(i)
			for(j=0;j<=ins->phase[i].codecn;++j){
				if(ins->phase[i].codec[j].cbinit)
					ins->phase[i].codec[j].cbinit(&(ins->phase[i].codec[j]),ins->phase[i].codec[j].priv);
			}
		}
	}
}

int bsdconv_get_phase_index(struct bsdconv_instance *ins, int phasen){
	/*
	 * phase[0] is a place holder for INPUT
	 * real phases range is [1,len]=[1,phasen]
	 */
	/* logical new index = len */
	if(phasen /* logical */ >= ins->phasen /* len */){
		/* real  = logical + 1 */
		phasen = ins->phasen + 1;
	}else{
		/* real  = (n + len) % (len) + 1*/
		phasen = (phasen + ins->phasen) % (ins->phasen) + 1;
	}
	return phasen;
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
		codecn = ins->phase[phasen].codecn+1;
	}else{
		/* real  = (n + len) % (len) */
		codecn = (codecn + ins->phase[phasen].codecn+1) % (ins->phase[phasen].codecn+1);
	}
	return codecn;
}

int bsdconv_insert_phase(struct bsdconv_instance *ins, int phase_type, int phasen){
	int i;

	phasen=bsdconv_get_phase_index(ins, phasen);

	++ins->phasen;
	ins->phase=realloc(ins->phase, sizeof(struct bsdconv_phase) * (ins->phasen+1));

	for(i=ins->phasen /* shifted index */;i>phasen;--i){
		ins->phase[i]=ins->phase[i-1];
	}
	ins->phase[phasen].type=phase_type;
	ins->phase[phasen].codec=malloc(sizeof(struct bsdconv_codec_t)*8);
	ins->phase[phasen].codecn=-1 /* trimmed length */;

	ins->phase[phasen].data_head=malloc(sizeof(struct data_rt));
	ins->phase[phasen].data_head->next=NULL;
	ins->phase[phasen].data_head->flags=0;

	return phasen;
}

int bsdconv_insert_codec(struct bsdconv_instance *ins, char *codec, int phasen, int codecn){
	int i;

	codecn=bsdconv_get_codec_index(ins, phasen, codecn);

	++ins->phase[phasen].codecn;
	ins->phase[phasen].codec=realloc(ins->phase[phasen].codec, sizeof(struct bsdconv_codec_t)*(ins->phase[phasen].codecn+1));

	for(i=ins->phase[phasen].codecn;i>codecn;--i){
		ins->phase[phasen].codec[i]=ins->phase[phasen].codec[i-1];
	}
	if(loadcodec(&ins->phase[phasen].codec[codecn], ins->phase[phasen].type, codec)<0){
		return -1;
	}
	if(ins->phase[phasen].codec[codecn].cbcreate)
		ins->phase[phasen].codec[codecn].priv=ins->phase[phasen].codec[codecn].cbcreate();
	return codecn;
}

struct bsdconv_instance *bsdconv_create(const char *conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	char *t, *t1, *t2;
	int i, j, f=0;

	i=1;
	for(t=(char *)conversion;*t;t++){
		if(*t==':' || *t=='|')++i;
	}
	ins->phasen=i;
	char *opipe[i+1];

	ins->phase=malloc(sizeof(struct bsdconv_phase) * (i+1));
	t2=t1=t=strdup(conversion);
//errorlevel 0

	i=1;
	while((t1=strsep(&t, "|")) != NULL){
		if(f>1){
			ins->phase[i-f].type=FROM;
			ins->phase[i-1].type=TO;
		}
		f=0;
		while((opipe[i]=strsep(&t1, ":"))!=NULL){
			ins->phase[i].type=INTER;
			i+=1;
			f+=1;
		}
	}
	if(f>1){
		ins->phase[i-f].type=FROM;
		ins->phase[i-1].type=TO;
	}
	ins->phase[0].type=INPUT;

	for(i=1;i<=ins->phasen;++i){
		if(*opipe[i]){
			ins->phase[i].codecn=1;
			for(t=(char *)opipe[i];*t;t++){
				if(*t==','){
					ins->phase[i].codecn+=1;
				}
			}
		}else{
			SetLastError(EINVAL);
			goto bsdconv_create_error_0;
		}
		ins->phase[i].codecn-=1;
	}

	for(i=1;i<=ins->phasen;++i){
		ins->phase[i].codec=malloc((ins->phase[i].codecn + 1)* sizeof(struct bsdconv_codec_t));
//errorlevel 1
	}
	for(i=1;i<=ins->phasen;++i){
		t=opipe[i];
		for(j=0;j<=ins->phase[i].codecn;++j){
			ins->phase[i].codec[j].desc=strsep(&t, ",");
			if(ins->phase[i].codec[j].desc[0]==0){
				SetLastError(EINVAL);
				goto bsdconv_create_error_1;
			}
		}
	}
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(!loadcodec(&ins->phase[i].codec[j], ins->phase[i].type, ins->phase[i].codec[j].desc)){
//errorlevel 2
				goto bsdconv_create_error_2;
			}
		}
	}
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(ins->phase[i].codec[j].cbcreate){
				ins->phase[i].codec[j].priv=ins->phase[i].codec[j].cbcreate();
			}
		}
	}
	for(i=0;i<=ins->phasen;++i){
		ins->phase[i].data_head=malloc(sizeof(struct data_rt));
		ins->phase[i].data_head->next=NULL;
		ins->phase[i].data_head->flags=0;
	}

	free(t2);

	ins->pool=NULL;
	return ins;
	bsdconv_create_error_2:
		free(ins->phase[i].codec[j].desc);
		j-=1;
		for(;i>=1;j=ins->phase[--i].codecn){
			for(;j>=0;--j){
				free(ins->phase[i].codec[j].desc);
				unloadcodec(&ins->phase[i].codec[j]);
			}
		}
	bsdconv_create_error_1:
		for(i=1;i<=ins->phasen;++i){
			free(ins->phase[i].codec);
		}
	bsdconv_create_error_0:
		free(t2);
		free(ins->phase);
		free(ins);
		return NULL;
}

void bsdconv_destroy(struct bsdconv_instance *ins){
	int i,j;
	struct data_rt *data_ptr;

	for(i=0;i<=ins->phasen;++i){
		if(i>0){
			for(j=0;j<=ins->phase[i].codecn;++j){
				free(ins->phase[i].codec[j].desc);
				if(ins->phase[i].codec[j].cbdestroy){
					ins->phase[i].codec[j].cbdestroy(ins->phase[i].codec[j].priv);
				}
				unloadcodec(&ins->phase[i].codec[j]);
			}
			free(ins->phase[i].codec);
		}
		while(ins->phase[i].data_head){
			data_ptr=ins->phase[i].data_head;
			ins->phase[i].data_head=ins->phase[i].data_head->next;
			if(data_ptr->flags & F_FREE)
				free(data_ptr->data);
			free(data_ptr);
		}
	}
	while(ins->pool){
		data_ptr=ins->pool;
		ins->pool=ins->pool->next;
		free(data_ptr);
	}
	free(ins->phase);
	free(ins);
}

void bsdconv(struct bsdconv_instance *ins){
	uintptr_t i;
	struct data_rt *data_ptr;
	char *ptr;
	FILE *fp;
	int fd;
	struct bsdconv_phase *this_phase;
	struct bsdconv_phase *prev_phase;

	if(ins->input.data!=NULL){
		DATA_MALLOC(ins->phase[0].data_tail->next);
		ins->phase[0].data_tail=ins->phase[0].data_tail->next;
		*(ins->phase[0].data_tail)=ins->input;
		ins->phase[0].data_tail->next=NULL;
		ins->input.data=NULL;
		ins->input.len=0;
		ins->input.flags=0;
	}

	ins->phase_index=1;

	phase_begin:
	if(ins->phase_index>0 && ins->phase_index<=ins->phasen){
		this_phase=&ins->phase[ins->phase_index];
		prev_phase=&ins->phase[ins->phase_index-1];
		switch(this_phase->type){
			case FROM:
				while(this_phase->data->next){
					if(this_phase->data == prev_phase->data_head) this_phase->i=this_phase->data_head->len;
					else this_phase->i=0;
					fflush(stdout);
					this_phase->data=this_phase->data->next;
					while(this_phase->i<this_phase->data->len){
						memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[UCP(this_phase->data->data)[this_phase->i]], sizeof(struct state_st));
						from_x:
						switch(this_phase->state.status){
							case DEADEND:
								from_deadend:
								this_phase->pend=0;
								if(this_phase->match){
									LISTCPY(this_phase->data_tail, this_phase->match, this_phase->codec[this_phase->index].data_z);

									LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
									this_phase->data=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len;
									this_phase->match=0;
									RESET(ins->phase_index);
									goto phase_begin;
								}else if(this_phase->index < this_phase->codecn){
									this_phase->index++;
									memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
									this_phase->data=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len;
									continue;
								}else{
									ins->ierr++;

									RESET(ins->phase_index);

									this_phase->bak=this_phase->data;
									LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
									this_phase->bak=this_phase->data=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len=this_phase->data_head->len+1;
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
								this_phase->data_head->len=this_phase->i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;

							case SUBMATCH:
								this_phase->match=this_phase->state.data;

								this_phase->bak=this_phase->data;
								this_phase->data_head->len=this_phase->i+1;

								this_phase->pend=1;
								break;
							case SUBROUTINE:
								this_phase->codec[this_phase->index].callback(ins);
								goto from_x;
							case NEXTPHASE:
								this_phase->match=0;
								this_phase->pend=0;

								this_phase->bak=this_phase->data;
								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->data=prev_phase->data_head;
								this_phase->data_head->len=this_phase->i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;
							case PASSTHRU:
								this_phase->bak=this_phase->data->next;
								while(prev_phase->data_head->next!=this_phase->data){
									data_ptr=prev_phase->data_head->next->next;
									DATA_FREE(prev_phase->data_head->next);
									prev_phase->data_head->next=data_ptr;
								}
								this_phase->data_tail->next=prev_phase->data_head->next;
								if(prev_phase->data_tail==prev_phase->data_head->next){
									prev_phase->data_tail=prev_phase->data_head;
								}
								prev_phase->data_head->next=prev_phase->data_head->next->next;
								this_phase->data_tail=this_phase->data_tail->next;
								this_phase->data_tail->next=NULL;

								this_phase->data_head->len=0;
								this_phase->data=prev_phase->data_head;

								this_phase->match=0;
								this_phase->pend=0;
								RESET(ins->phase_index);
								ins->phase_index+=1;
								goto phase_begin;
							case CONTINUE:
								this_phase->pend=1;
								break;
							case NOOP:
								goto phase_begin;
						}
						this_phase->i+=1;
					}
				}
			break;

		case INTER:
			while(this_phase->data->next){
				this_phase->data=this_phase->data->next;
				this_phase->state.status=DUMMY;
				for(this_phase->i=0;this_phase->i<this_phase->data->len;this_phase->i+=1){
					memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[UCP(this_phase->data->data)[this_phase->i]], sizeof(struct state_st));
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
						ins->phase_index+=1;
						goto phase_begin;
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
							DATA_MALLOC(prev_phase->data_tail->next);
							this_phase->bak=prev_phase->data_tail->next;
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->flags=0;
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
				for(this_phase->i=0;this_phase->i<this_phase->data->len;this_phase->i+=1){
					memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.sub[UCP(this_phase->data->data)[this_phase->i]], sizeof(struct state_st));
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
							DATA_MALLOC(prev_phase->data_tail->next);
							this_phase->bak=prev_phase->data_tail->next;
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->flags=0;
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
					case PASSTHRU:
						this_phase->bak=this_phase->data->next;
						while(prev_phase->data_head->next!=this_phase->data){
							data_ptr=prev_phase->data_head->next->next;
							DATA_FREE(prev_phase->data_head->next);
							prev_phase->data_head->next=data_ptr;
						}
						this_phase->data_tail->next=prev_phase->data_head->next;
						if(prev_phase->data_tail==prev_phase->data_head->next){
							prev_phase->data_tail=prev_phase->data_head;
						}
						prev_phase->data_head->next=prev_phase->data_head->next->next;
						this_phase->data_tail=this_phase->data_tail->next;
						this_phase->data_tail->flags |= F_SKIP;
						this_phase->data_tail->next=NULL;

						this_phase->data_head->len=0;
						this_phase->data=prev_phase->data_head;

						this_phase->match=0;
						this_phase->pend=0;
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
			ins->output.flags=0;
			break;
		case BSDCONV_AUTOMALLOC:
			i=ins->output.len;
			data_ptr=ins->phase[ins->phasen].data_head;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
			ins->output.flags=1;
			ptr=ins->output.data=malloc(i);
			ins->output.len=i-ins->output.len;
			data_ptr=ins->phase[ins->phasen].data_head;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				memcpy(ptr, data_ptr->data, data_ptr->len);
				ptr+=data_ptr->len;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			break;
		case BSDCONV_PREMALLOCED:
			ins->output.flags=0;
			if(ins->output.data!=NULL && ins->output.len){
				i=0;
				while(ins->phase[ins->phasen].data_head->next && ins->phase[ins->phasen].data_head->next->len<=ins->output.len-i){
					memcpy(ins->output.data+i, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
					i+=ins->phase[ins->phasen].data_head->next->len;
					if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
						ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
					}
					data_ptr=ins->phase[ins->phasen].data_head->next;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					DATA_FREE(data_ptr);
				}
				ins->output.len=i;
			}else{
				i=0;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ins->output.len=i;
			}
			break;
		case BSDCONV_FILE:
			fp=ins->output.data;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				fwrite(data_ptr->data, data_ptr->len, 1, fp);
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
			break;
		case BSDCONV_FD:
			fd=(intptr_t)ins->output.data;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				write(fd, data_ptr->data, data_ptr->len);
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
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
