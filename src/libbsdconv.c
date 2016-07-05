#define USE_HEX_MAP

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
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#ifndef MAP_PREFAULT_READ
#define MAP_PREFAULT_READ 0
#endif

#ifdef WIN32
#define MODULES_SUBPATH "modules"
#else
#define MODULES_SUBPATH "share/bsdconv"
#endif

struct bsdconv_instance *bsdconv_unpack(const char *);
char *bsdconv_pack(struct bsdconv_instance *);

#include "libbsdconv_counter.c"
#include "libbsdconv_filter.c"
#include "libbsdconv_scorer.c"
#include "libbsdconv_hash.c"
#include "libbsdconv_module.c"
#include "libbsdconv_util.c"

static inline int _cbcreate(struct bsdconv_instance *ins, int p, int c){
	int r;
	char *argv;
	if(ins->phase[p].codec[c].argv)
		argv=strdup(ins->phase[p].codec[c].argv);
	else
		argv=strdup("");
	char *cur=argv;
	char *k;
	struct bsdconv_hash_entry *arg=NULL, *tmp;
	struct bsdconv_hash_entry **last=&arg;
	if(*cur){
		while((k=strsep(&cur, "&"))!=NULL){
			*last=malloc(sizeof(struct bsdconv_hash_entry));
			(*last)->key=k;
			(*last)->ptr=strchr(k, '=');
			if((*last)->ptr){
				*CP((*last)->ptr)=0;
				(*last)->ptr+=1;
			}
			(*last)->next=NULL;
			last=&((*last)->next);
		}
	}
	r=ins->phase[p].codec[c].cbcreate(ins, arg);
	free(argv);
	while(arg){
		tmp=arg->next;
		free(arg);
		arg=tmp;
	}
	return r;
}

int _loadcodec(struct bsdconv_codec *cd, char *path){
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
	cd->cbctl=NULL;
	cd->cbconv=NULL;
	cd->cbflush=NULL;
	cd->cbdestroy=NULL;
	strcat(path, "." SHLIBEXT);

	if((cd->dl=OPEN_SHAREOBJECT(path))){
		cd->cbconv=SHAREOBJECT_SYMBOL(cd->dl,"cbconv");
		cd->cbflush=SHAREOBJECT_SYMBOL(cd->dl,"cbflush");
		cd->cbcreate=SHAREOBJECT_SYMBOL(cd->dl,"cbcreate");
		cd->cbinit=SHAREOBJECT_SYMBOL(cd->dl,"cbinit");
		cd->cbctl=SHAREOBJECT_SYMBOL(cd->dl,"cbctl");
		cd->cbdestroy=SHAREOBJECT_SYMBOL(cd->dl,"cbdestroy");
	}

	return 1;
}

int loadcodec(struct bsdconv_codec *cd, int type){
	char *cwd;
	char *c;
	char buf[PATH_MAX+1];
	char *upper;
	cwd=getcwd(NULL, 0);
	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}
	chdir(MODULES_SUBPATH);
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
	upper=strdup(cd->desc);
	strtoupper(upper);
	REALPATH(upper, buf);
	chdir(cwd);
	free(cwd);
	free(upper);
	if(!_loadcodec(cd, buf))
		return 0;
	return 1;
}

void unloadcodec(struct bsdconv_codec *cd){
	if(cd->dl){
		CLOSE_SHAREOBJECT(cd->dl);
	}
#ifdef WIN32
	UnmapViewOfFile(cd->z);
	CloseHandle(cd->md);
	CloseHandle(cd->fd);
#else
	munmap(cd->z, cd->maplen);
	close(cd->fd);
#endif
}

void bsdconv_init(struct bsdconv_instance *ins){
	int i, j;
	struct data_rt *data_ptr;

	ins->flush=0;
	ins->input.data=NULL;
	ins->input.flags=0;
	ins->input.len=0;
	ins->output.data=NULL;
	ins->output.len=0;
	ins->output_mode=BSDCONV_HOLD;

	for(i=0;i<=ins->phasen;++i){
		ins->phase_index=i;
		ins->phase[i].flags=0;
		ins->phase[i].offset=0;
		while(ins->phase[i].data_head->next){
			data_ptr=ins->phase[i].data_head->next;
			ins->phase[i].data_head->next=ins->phase[i].data_head->next->next;
			if(data_ptr->flags & F_FREE)
				free(data_ptr->data);
			free(data_ptr);
		}
		ins->phase[i].data_tail=ins->phase[i].data_head;
		ins->phase[i].data_head->len=0;
		ins->phase[i].match_data=NULL;
		if(i>0){
			ins->phase[i].curr=ins->phase[i-1].data_head;
			for(j=0;j<=ins->phase[i].codecn;++j){
				ins->phase[i].index=j;
				if(ins->phase[i].codec[j].cbinit)
					ins->phase[i].codec[j].cbinit(ins);
			}
			RESET(i);
		}
	}
}

void bsdconv_ctl(struct bsdconv_instance *ins, int ctl, void *p, int v){
	int i, j;
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(ins->phase[i].codec[j].cbctl){
				ins->phase_index=i;
				ins->phase[i].index=j;
				ins->phase[i].codec[j].cbctl(ins, ctl, p, v);
			}
		}
	}
}

char *bsdconv_pack(struct bsdconv_instance *ins){
	char *ret;
	char *t, *cur;
	const char *end;
	int len=0;
	int i, j, n;
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			len+=strlen(ins->phase[i].codec[j].desc);
			t=ins->phase[i].codec[j].desc;
			n=1;
			while(*t){
				if(*t==',')
					n+=1;
				t++;
			}
			if(ins->phase[i].codec[j].argv)
				len+=(strlen(ins->phase[i].codec[j].argv)+1)*n;
			len+=1;
		}
	}
	ret=malloc(sizeof(char) * len);
	ret[0]=0;
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(j==0){
				if(i>1){
					switch(ins->phase[i].type){
						case FROM:
							strcat(ret, "|");
							break;
						case INTER:
						case TO:
							strcat(ret, ":");
						break;
					}
				}
			}else{
				strcat(ret, ",");
			}
			t=ins->phase[i].codec[j].desc;
			while(1){
				cur=strchr(t, ',');
				if(cur){
					end=",";
					*cur=0;
				}else{
					end="";
				}
				strcat(ret, t);
				if(ins->phase[i].codec[j].argv && *(ins->phase[i].codec[j].argv)){
					if(strchr(t,'#')==NULL)
						strcat(ret, "#");
					else
						strcat(ret, "&");
					strcat(ret, ins->phase[i].codec[j].argv);
				}
				strcat(ret, end);
				if(cur)
					t=cur+1;
				else
					break;
			}
		}
	}
	return ret;
}

struct bsdconv_instance *bsdconv_unpack(const char *_conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	char *conversion;
	char *t, *t1;
	int i, j;
	int f=0;

	conversion=strdup(_conversion);
	t1=t=conversion;
	i=1;
	for(t=(char *)conversion;*t;t++){
		if(*t==':' || *t=='|')++i;
	}
	ins->phasen=i;
	char *opipe[i+1];

	ins->phase=malloc(sizeof(struct bsdconv_phase) * (i+1));

	i=1;
	t1=t=conversion;
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
	ins->phase[0].type=_INPUT;

	for(i=1;i<=ins->phasen;++i){
		if(*opipe[i]){
			ins->phase[i].codecn=1;
			for(t=(char *)opipe[i];*t;t++){
				if(*t==','){
					ins->phase[i].codecn+=1;
				}
			}
		}else{
			free(ins->phase);
			free(ins);
			free(conversion);
			return NULL;
		}
		ins->phase[i].codecn-=1;
	}
	for(i=1;i<=ins->phasen;++i){
		ins->phase[i].codec=malloc((ins->phase[i].codecn + 1)* sizeof(struct bsdconv_codec));
	}
	for(i=1;i<=ins->phasen;++i){
		t=opipe[i];
		for(j=0;j<=ins->phase[i].codecn;++j){
			ins->phase[i].codec[j].desc=strdup(strsep(&t, ","));
			ins->phase[i].codec[j].argv=strchr(ins->phase[i].codec[j].desc, '#');
			if(ins->phase[i].codec[j].argv){
				*(ins->phase[i].codec[j].argv)=0;
				ins->phase[i].codec[j].argv+=1;
			}
			if(ins->phase[i].codec[j].desc[0]==0){
				for(;j>=0;--j){
					free(ins->phase[i].codec[j].desc);
				}
				for(i=1;i<=ins->phasen;++i){
					free(ins->phase[i].codec);
				}
				free(ins->phase);
				free(ins);
				free(conversion);
				return NULL;
			}
		}
	}
	free(conversion);
	return ins;
}

struct bsdconv_instance *bsdconv_create(const char *_conversion){
	int e=0;
	struct bsdconv_instance *ins=NULL;
	char *conversion=malloc(strlen(_conversion)+1);
	int i, j;
	char *c;
	const char *d;
	char whitespace[256]={0};
	whitespace['\r']=1;
	whitespace['\n']=1;
	whitespace['\t']=1;
	whitespace['\f']=1;
	whitespace[' ']=1;
	d=_conversion;
	c=conversion;
	while(*d){
		if(whitespace[*UCP(d)]==0){
			*c=*d;
			c+=1;
		}
		d+=1;
	}
	*c=0;

	i=0;
	while(i==0 || i<=ins->phasen){
		start_parse:
		ins=bsdconv_unpack(conversion);
		if(ins==NULL){
			free(conversion);
			SetLastError(EINVAL);
			return NULL;
		}
		for(i=1;i<=ins->phasen;++i){
			for(j=0;j<=ins->phase[i].codecn;++j){
				if(!bsdconv_module_check(ins->phase[i].type, ins->phase[i].codec[j].desc)){
					c=bsdconv_solve_alias(ins->phase[i].type, ins->phase[i].codec[j].desc);
					if(c==NULL){
						e=1;
					}else{
						if(strcmp(c, ins->phase[i].codec[j].desc)==0)
							e=1;
						free(ins->phase[i].codec[j].desc);
						ins->phase[i].codec[j].desc=c;
					}
					free(conversion);
					conversion=bsdconv_pack(ins);
					for(i=1;i<=ins->phasen;++i){
						for(j=0;j<=ins->phase[i].codecn;++j){
							free(ins->phase[i].codec[j].desc);
						}
						free(ins->phase[i].codec);
					}
					free(ins->phase);
					free(ins);
					if(e){
						SetLastError(EOPNOTSUPP);
						free(conversion);
						return NULL;
					}
					goto start_parse;
				}
			}
		}
	}
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(!loadcodec(&ins->phase[i].codec[j], ins->phase[i].type)){
				free(ins->phase[i].codec[j].desc);
				j-=1;
				for(;i>=1;j=ins->phase[--i].codecn){
					for(;j>=0;--j){
						free(ins->phase[i].codec[j].desc);
						unloadcodec(&ins->phase[i].codec[j]);
					}
				}
				goto bsdconv_create_error;
			}
		}
	}

	ins->pool=NULL;
	ins->hash=NULL;
	ins->counter=NULL;
	ins->input.flags=0;
	ins->output.flags=0;

	ins->ierr=bsdconv_counter(ins, "IERR");
	ins->oerr=bsdconv_counter(ins, "OERR");

	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(ins->phase[i].codec[j].cbcreate){
				ins->phase_index=i;
				ins->phase[i].index=j;
				e=_cbcreate(ins, i, j);
				if(e){
					for(j=j-1;j>=0;j-=1){
						if(ins->phase[i].codec[j].cbdestroy){
							ins->phase_index=i;
							ins->phase[i].index=j;
							ins->phase[i].codec[j].cbdestroy(ins);
						}
					}
					for(i=i-1;i>=1;i-=1){
						for(j=0;j<=ins->phase[i].codecn;++j){
							if(ins->phase[i].codec[j].cbdestroy){
								ins->phase_index=i;
								ins->phase[i].index=j;
								ins->phase[i].codec[j].cbdestroy(ins);
							}
						}
					}
					for(i=1;i<=ins->phasen;++i){
						for(j=0;j<=ins->phase[i].codecn;++j){
							free(ins->phase[i].codec[j].desc);
							unloadcodec(&ins->phase[i].codec[j]);
						}
					}
					SetLastError(e);
					goto bsdconv_create_error;
				}
			}
		}
	}
	for(i=0;i<=ins->phasen;++i){
		ins->phase[i].data_head=malloc(sizeof(struct data_rt));
		ins->phase[i].data_head->next=NULL;
		ins->phase[i].data_head->flags=0;
	}

	free(conversion);
	return ins;

bsdconv_create_error:
	for(i=1;i<=ins->phasen;++i){
		free(ins->phase[i].codec);
	}

	free(conversion);
	free(ins->phase);

	void *p;
	while(ins->hash){
		free(ins->hash->key);
		p=ins->hash->next;
		free(ins->hash);
		ins->hash=p;
	}
	while(ins->counter){
		free(ins->counter->key);
		p=ins->counter->next;
		free(ins->counter);
		ins->counter=p;
	}
	free(ins);
	return NULL;
}

void bsdconv_destroy(struct bsdconv_instance *ins){
	int i, j;
	struct data_rt *data_ptr;
	void *p;

	for(i=0;i<=ins->phasen;++i){
		if(i>0){
			for(j=0;j<=ins->phase[i].codecn;++j){
				free(ins->phase[i].codec[j].desc);
				if(ins->phase[i].codec[j].cbdestroy){
					ins->phase_index=i;
					ins->phase[i].index=j;
					ins->phase[i].codec[j].cbdestroy(ins);
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
	while(ins->hash){
		free(ins->hash->key);
		p=ins->hash->next;
		free(ins->hash);
		ins->hash=p;
	}
	while(ins->counter){
		free(ins->counter->key);
		p=ins->counter->next;
		free(ins->counter);
		ins->counter=p;
	}
	free(ins);
}

void bsdconv(struct bsdconv_instance *ins){
	struct bsdconv_instance *inso;
	uintptr_t i;
	struct data_rt *data_ptr;
	char *ptr;
	FILE *fp;
	int fd;
	unsigned char c;
	struct bsdconv_phase *prev_phase;
	struct bsdconv_phase *this_phase;
	struct bsdconv_codec *this_codec;

	if(ins->input.data!=NULL){
		DATA_MALLOC(ins, ins->phase[0].data_tail->next);
		ins->phase[0].data_tail=ins->phase[0].data_tail->next;
		*(ins->phase[0].data_tail)=ins->input;
		ins->input.data=NULL;
		ins->input.len=0;
		ins->input.flags=0;
	}

	ins->phase_index=1;

	phase_begin:
	if(ins->phase_index>0 && ins->phase_index<=ins->phasen){
		prev_phase=PREV_PHASE(ins);
		this_phase=THIS_PHASE(ins);
		this_codec=THIS_CODEC(ins);
		switch(this_phase->type){
			case FROM:
				while(this_phase->curr->next){
					if(this_phase->curr == prev_phase->data_head) this_phase->i=this_phase->data_head->len;
					else this_phase->i=0;
					this_phase->curr=this_phase->curr->next;
					while(this_phase->i<this_phase->curr->len){
						c=UCP(this_phase->curr->data)[this_phase->i];
						if(c>=this_phase->state.beg && c<this_phase->state.end)
							memcpy(&this_phase->offset, this_codec->z + (uintptr_t)this_phase->state.base + (c - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
						else if(!(this_phase->flags & F_LOOPBACK))
							this_phase->offset=0;
						this_phase->state=read_state(this_codec->z + this_phase->offset);
						from_x:
						switch(this_phase->state.status){
							case DEADEND:
								from_deadend:
								this_phase->flags &= ~(F_PENDING | F_LOOPBACK);
								if(this_phase->flags & F_MATCH){
									if(this_phase->match_data){
										LISTCPY_ST(ins, this_phase->data_tail, this_phase->match_data, this_codec->data_z);

										LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
										this_phase->curr=prev_phase->data_head;
										this_phase->i=this_phase->data_head->len;
									}else if(this_codec->cbflush){
										this_codec->cbflush(ins);
									}
									this_phase->flags &= ~F_MATCH;
									RESET(ins->phase_index);
									goto phase_begin;
								}else if(this_phase->index < this_phase->codecn){
									this_phase->index++;
									this_codec=THIS_CODEC(ins);

									this_phase->state=read_state(this_codec->z);

									this_phase->curr=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len;
									continue;
								}else{
									*(ins->ierr)+=1;

									RESET(ins->phase_index);
									this_codec=THIS_CODEC(ins);

									this_phase->bak=this_phase->curr;
									LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
									this_phase->bak=this_phase->curr=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len=this_phase->data_head->len+1;
									continue;
								}
								break;
							case MATCH:
								this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
								this_phase->match_data=NULL;

								LISTCPY_ST(ins, this_phase->data_tail, this_phase->state.data, this_codec->data_z);

								this_phase->bak=this_phase->curr;
								LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
								this_phase->data_head->len=this_phase->i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;
							case SUBMATCH:
								this_phase->flags |= (F_MATCH | F_PENDING);
								this_phase->match_data=this_phase->state.data;

								this_phase->bak=this_phase->curr;
								this_phase->data_head->len=this_phase->i+1;
								break;
							case SUBROUTINE:
							case SUBMATCH_SUBROUTINE:
								this_codec->cbconv(ins);
								this_phase->flags |= F_LOOPBACK;

								goto from_x;
							case YIELD:
								// to be investigated
								// this_phase->flags &= ~(F_MATCH | F_PENDING);
								this_phase->match_data=NULL;

								this_phase->bak=this_phase->curr;
								LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
								this_phase->data_head->len=this_phase->i;

								this_codec->cbconv(ins);
								this_phase->flags |= F_LOOPBACK;

								goto from_x;
							case NEXTPHASE:
								this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
								this_phase->match_data=NULL;

								this_phase->bak=this_phase->curr;
								LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
								this_phase->data_head->len=this_phase->i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;
							case CONTINUE:
								this_phase->flags |= F_PENDING;
								break;
							case NOOP:
								goto phase_begin;
						}
						this_phase->i+=1;
					}
				}
			break;

		case INTER:
			while(this_phase->curr->next){
				this_phase->curr=this_phase->curr->next;
				this_phase->state.status=NOMATCH;
				for(this_phase->i=0;this_phase->i<this_phase->curr->len;this_phase->i+=1){
					c=UCP(this_phase->curr->data)[this_phase->i];
					if(c>=this_phase->state.beg && c<this_phase->state.end){
						memcpy(&this_phase->offset, this_codec->z + (uintptr_t)this_phase->state.base + (c - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
					}else if(!(this_phase->flags & F_LOOPBACK)){
						this_phase->offset=0;
					}
					this_phase->state=read_state(this_codec->z + this_phase->offset);
					switch(this_phase->state.status){
						case DEADEND:
							goto inter_deadend;
							break;
						case SUBROUTINE:
						case SUBMATCH_SUBROUTINE:
							this_phase->flags |= F_LOOPBACK;
							break;
					}
				}
				inter_x:
				switch(this_phase->state.status){
					case NOMATCH:
						ins->phase_index+=1;
						goto phase_begin;
					case DEADEND:
						inter_deadend:
						this_phase->flags &= ~(F_PENDING | F_LOOPBACK);
						if(this_phase->flags & F_MATCH){
							if(this_phase->match_data){
								LISTCPY_ST(ins, this_phase->data_tail, this_phase->match_data, this_codec->data_z);

								LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
							}else if(this_codec->cbflush){
								this_codec->cbflush(ins);
							}

							this_phase->flags &= ~F_MATCH;
							RESET(ins->phase_index);
							goto phase_begin;
						}else if(this_phase->index < this_phase->codecn){
							this_phase->index++;
							this_codec=THIS_CODEC(ins);

							this_phase->state=read_state(this_codec->z);

							this_phase->curr=prev_phase->data_head;
							continue;
						}else{
							data_ptr=prev_phase->data_head->next;
							prev_phase->data_head->next=prev_phase->data_head->next->next;
							this_phase->curr=prev_phase->data_head;
							data_ptr->next=NULL;
							this_phase->data_tail->next=data_ptr;
							this_phase->data_tail=data_ptr;
							if(prev_phase->data_tail==data_ptr){
								prev_phase->data_tail=prev_phase->data_head;
							}

							RESET(ins->phase_index);

							ins->phase_index+=1;
							goto phase_begin;
						}
						break;
					case MATCH:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						LISTCPY_ST(ins, this_phase->data_tail, this_phase->state.data, this_codec->data_z);

						this_phase->bak=this_phase->curr->next;
						LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case SUBMATCH:
						this_phase->flags |= (F_MATCH | F_PENDING);
						this_phase->match_data=this_phase->state.data;

						if(this_phase->curr->next){
							this_phase->bak=this_phase->curr->next;
						}else{
							DATA_MALLOC(ins, prev_phase->data_tail->next);
							this_phase->bak=prev_phase->data_tail->next;
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->flags=0;
						}

						break;
					case SUBROUTINE:
					case SUBMATCH_SUBROUTINE:
						this_codec->cbconv(ins);
						goto inter_x;
					case YIELD:
						// to be investigated
						// this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						this_phase->bak=this_phase->curr;
						LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);

						this_codec->cbconv(ins);
						goto inter_x;
					case NEXTPHASE:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						this_phase->bak=this_phase->curr->next;
						LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case CONTINUE:
						this_phase->flags |= F_PENDING;
						break;
					case NOOP:
						goto phase_begin;

				}
				if(256<this_phase->state.end)
					memcpy(&this_phase->offset, this_codec->z + (uintptr_t)this_phase->state.base + (256 - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
				else if(!(this_phase->flags & F_LOOPBACK))
					this_phase->offset=0;
				this_phase->state=read_state(this_codec->z + this_phase->offset);
				if(this_phase->state.status==DEADEND){ goto inter_deadend;}
			}
			break;

		case TO:
			while(this_phase->curr->next){
				this_phase->curr=this_phase->curr->next;
				this_phase->state.status=NOMATCH;
				for(this_phase->i=0;this_phase->i<this_phase->curr->len;this_phase->i+=1){
					c=UCP(this_phase->curr->data)[this_phase->i];
					if(c>=this_phase->state.beg && c<this_phase->state.end)
						memcpy(&this_phase->offset, this_codec->z + (uintptr_t)this_phase->state.base + (c - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
					else if(!(this_phase->flags & F_LOOPBACK))
						this_phase->offset=0;
					this_phase->state=read_state(this_codec->z + this_phase->offset);
					switch(this_phase->state.status){
						case DEADEND:
							goto to_deadend;
							break;
						case SUBROUTINE:
						case SUBMATCH_SUBROUTINE:
							this_phase->flags |= F_LOOPBACK;
							break;
					}
				}
				to_x:
				switch(this_phase->state.status){
					case NOMATCH:
						continue;
					case DEADEND:
						to_deadend:
						this_phase->flags &= ~(F_PENDING | F_LOOPBACK);
						if(this_phase->flags & F_MATCH){
							if(this_phase->match_data){
								LISTCPY_ST(ins, this_phase->data_tail, this_phase->match_data, this_codec->data_z);

								LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
							}else if(this_codec->cbflush){
								this_codec->cbflush(ins);
							}

							this_phase->flags &= ~F_MATCH;
							RESET(ins->phase_index);
							ins->phase_index+=1;
							goto phase_begin;
						}else if(this_phase->index < this_phase->codecn){
							this_phase->index++;
							this_codec=THIS_CODEC(ins);

							this_phase->state=read_state(this_codec->z);

							this_phase->curr=prev_phase->data_head;
							continue;
						}else{
							*(ins->oerr)+=1;

							RESET(ins->phase_index);
							this_codec=THIS_CODEC(ins);

							this_phase->bak=this_phase->curr->next;
							LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
							this_phase->bak=this_phase->curr=prev_phase->data_head;

							continue;
						}
						break;
					case MATCH:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						LISTCPY_ST(ins, this_phase->data_tail, this_phase->state.data, this_codec->data_z);

						this_phase->bak=this_phase->curr->next;
						LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);
						ins->phase_index+=1;
						goto phase_begin;
					case SUBMATCH:
						this_phase->flags |= (F_MATCH | F_PENDING);
						this_phase->match_data=this_phase->state.data;
						if(this_phase->curr->next){
							this_phase->bak=this_phase->curr->next;
						}else{
							DATA_MALLOC(ins, prev_phase->data_tail->next);
							this_phase->bak=prev_phase->data_tail->next;
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->flags=0;
						}

						break;
					case SUBROUTINE:
					case SUBMATCH_SUBROUTINE:
						this_codec->cbconv(ins);
						goto to_x;
					case YIELD:
						// to be investigated
						//this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						this_phase->bak=this_phase->curr;
						LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);

						this_codec->cbconv(ins);
						goto to_x;
					case NEXTPHASE:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						this_phase->bak=this_phase->curr->next;
						LISTFREE(ins, prev_phase->data_head, this_phase->bak, prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case CONTINUE:
						this_phase->flags|=F_PENDING;
						this_phase->flags &= ~F_LOOPBACK;
						break;
					case NOOP:
						goto phase_begin;
				}
				if(256<this_phase->state.end)
					memcpy(&this_phase->offset, this_codec->z + (uintptr_t)this_phase->state.base + (256 - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
				else if(!(this_phase->flags & F_LOOPBACK))
					this_phase->offset=0;
				this_phase->state=read_state(this_codec->z + this_phase->offset);
				if(this_phase->state.status==DEADEND){ goto to_deadend;}
			}
			break;
		}
		ins->phase_index+=1;
	}

	//check back (phase-loop)
	for(ins->phase_index=ins->phasen;ins->phase_index>0;ins->phase_index-=1){
		if(ins->phase[ins->phase_index].curr->next){
			goto phase_begin;
		}
	}

	//flush
	if(ins->flush){
		for(ins->phase_index=1;ins->phase_index<=ins->phasen;++(ins->phase_index)){
			if(THIS_PHASE(ins)->flags & F_PENDING){
				prev_phase=PREV_PHASE(ins);
				this_phase=THIS_PHASE(ins);
				this_codec=THIS_CODEC(ins);
				switch(this_phase->type){
					case FROM:	goto from_deadend;
					case INTER:	goto inter_deadend;
					case TO:	goto to_deadend;
				}
			}
		}
	}

	struct bsdconv_phase *last_phase =  LAST_PHASE(ins);
	//output
	switch(ins->output_mode){
		case BSDCONV_HOLD:
			ins->output.len=0;
			ins->output.flags=0;
			break;
		case BSDCONV_AUTOMALLOC:
			i=ins->output.len;
			data_ptr=last_phase->data_head->next;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			last_phase->data_tail=last_phase->data_head;
			ins->output.flags=1;
			ptr=ins->output.data=malloc(i);
			ins->output.len=i-ins->output.len;
			data_ptr=last_phase->data_head;
			while(last_phase->data_head->next){
				data_ptr=last_phase->data_head->next;
				memcpy(ptr, data_ptr->data, data_ptr->len);
				ptr+=data_ptr->len;
				last_phase->data_head->next=last_phase->data_head->next->next;
				DATUM_FREE(ins, data_ptr);
			}
			break;
		case BSDCONV_PREMALLOCED:
			ins->output.flags=0;
			if(ins->output.data!=NULL && ins->output.len){
				i=0;
				while(last_phase->data_head->next && last_phase->data_head->next->len<=ins->output.len-i){
					memcpy(ins->output.data+i, last_phase->data_head->next->data, last_phase->data_head->next->len);
					i+=last_phase->data_head->next->len;
					if(last_phase->data_tail==last_phase->data_head->next){
						last_phase->data_tail=last_phase->data_head;
					}
					data_ptr=last_phase->data_head->next;
					last_phase->data_head->next=last_phase->data_head->next->next;
					DATUM_FREE(ins, data_ptr);
				}
				ins->output.len=i;
			}else{
				i=0;
				data_ptr=last_phase->data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ins->output.len=i;
			}
			break;
		case BSDCONV_FILE:
			fp=ins->output.data;
			while(last_phase->data_head->next){
				data_ptr=last_phase->data_head->next;
				fwrite(data_ptr->data, data_ptr->len, 1, fp);
				last_phase->data_head->next=last_phase->data_head->next->next;
				DATUM_FREE(ins, data_ptr);
			}
			last_phase->data_tail=last_phase->data_head;
			break;
		case BSDCONV_FD:
			fd=(intptr_t)ins->output.data;
			while(last_phase->data_head->next){
				data_ptr=last_phase->data_head->next;
				write(fd, data_ptr->data, data_ptr->len);
				last_phase->data_head->next=last_phase->data_head->next->next;
				DATUM_FREE(ins, data_ptr);
			}
			last_phase->data_tail=last_phase->data_head;
			break;
		case BSDCONV_NULL:
			while(last_phase->data_head->next){
				data_ptr=last_phase->data_head->next;
				last_phase->data_head->next=last_phase->data_head->next->next;
				DATUM_FREE(ins, data_ptr);
			}
			last_phase->data_tail=last_phase->data_head;
			break;
		case BSDCONV_PASS:
			inso=ins->output.data;
			if(last_phase->data_head->next){
				inso->input=*(last_phase->data_head->next);
				data_ptr=last_phase->data_head->next->next;
				last_phase->data_head->next->flags &= ~F_FREE;
				DATUM_FREE(ins, last_phase->data_head->next);
				last_phase->data_head->next=data_ptr;
			}
			struct data_rt *tail;
			tail=&inso->input;
			while(last_phase->data_head->next){
				tail->next=dup_data_rt(inso, last_phase->data_head->next);
				tail=tail->next;
				data_ptr=last_phase->data_head->next->next;
				DATUM_FREE(ins, last_phase->data_head->next);
				last_phase->data_head->next=data_ptr;
			}
			last_phase->data_tail=last_phase->data_head;
			break;
	}
	return;
}

char * bsdconv_error(void){
	switch(GetLastError()){
		case EDOOFUS:
				return strdup("Unexpected condition");
		case EOPNOTSUPP:
				return strdup("Unsupported charset/encoding or filter");
		case ENOMEM:
				return strdup("Mmap failed");
		case EINVAL:
				return strdup("Conversion syntax error");
		default:
				return strdup("Unknown error");
	}
}
