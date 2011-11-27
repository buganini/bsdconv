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

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include "bsdconv.h"
#include "fmalloc.h"
#ifndef WIN32
#include <errno.h>
#endif

#ifdef USE_FMALLOC
extern const char *fmalloc_template;
#endif

struct m_data_st{
	char *data;
	size_t len;
	offset_t next;

	offset_t offset;
	struct m_data_st *n;
};

struct m_state_st{
	char status;
	struct m_data_st *data;
	uint16_t beg;
	uint16_t end;
	struct m_state_st **base;

	int prio;

	offset_t offset;
	struct m_state_st *n;
};

void STATE_INIT(struct m_state_st *state_t){
	state_t->n=NULL;
	state_t->beg=~0;
	state_t->end=0;
	state_t->base=NULL;
	state_t->data=NULL;
}

struct list{
	struct m_state_st *p;
	int u,l,pr;
	struct list *n;
};

struct dhash{
	uintptr_t v;
	int c;
	struct dhash *p;
	struct dhash **sub;
	offset_t offset;
	void *head;
};

char table[256]={};
char ci_table[256]={0};

uintptr_t offset=0;
struct m_state_st *state_r, *state_t, holder;

struct dhash *hash_datalist, *hash_data;

uintptr_t hash(int *p, uintptr_t l){
	int i;
	struct dhash *hash_p=hash_datalist;
	struct dhash *hash_q=hash_data;
	for(i=l-1;i>=0;--i){
		if(hash_p->sub==NULL)
			hash_p->sub=calloc(257, sizeof(struct dhash *));
		if(hash_p->sub[p[i]]==NULL){
			hash_p->sub[p[i]]=FMALLOC(sizeof(struct dhash));
			hash_p->sub[p[i]]->c=p[i];
			hash_p->sub[p[i]]->p=hash_p;
			hash_p->sub[p[i]]->v=0;
			hash_p->sub[p[i]]->offset=0;
			hash_p->sub[p[i]]->sub=NULL;
		}
		//printf("hash_p: [%d]%X %p => %p\n", i, p[i], hash_p, hash_p->sub[p[i]]);
		if(p[i]==256){
			hash_p->v=(uintptr_t) hash_q;
			hash_p=hash_p->sub[p[i]];
			hash_q=hash_data;
		}else{
			hash_p=hash_p->sub[p[i]];
			if(hash_q->sub==NULL)
				hash_q->sub=calloc(257, sizeof(struct dhash *));
			if(hash_q->sub[p[i]]==NULL){
				hash_q->sub[p[i]]=FMALLOC(sizeof(struct dhash));
				hash_q->sub[p[i]]->c=p[i];
				hash_q->sub[p[i]]->p=hash_q;
				hash_q->sub[p[i]]->v=0;
				hash_q->sub[p[i]]->offset=0;
				hash_q->sub[p[i]]->head=NULL;
				hash_q->sub[p[i]]->sub=NULL;
			}
			hash_q=hash_q->sub[p[i]];
		}
	}
	hash_p->v=(uintptr_t) hash_q;
	return (uintptr_t) hash_p;
}
struct dhash *callback_hash;

uintptr_t hash_callback(int *p, uintptr_t l){
	int i;
	struct dhash *hash_p=hash_datalist;
	struct dhash *hash_q=callback_hash;
	for(i=l-1;i>=0;--i){
		hash_p=hash_p->sub[p[i]];
		if(hash_q->sub==NULL)
			hash_q->sub=calloc(257, sizeof(struct dhash *));
		if(hash_q->sub[p[i]]==NULL){
			hash_q->sub[p[i]]=FMALLOC(sizeof(struct dhash));
			hash_q->sub[p[i]]->c=p[i];
			hash_q->sub[p[i]]->p=hash_q;
			hash_q->sub[p[i]]->v=0;
			hash_q->sub[p[i]]->offset=0;
			hash_q->sub[p[i]]->head=NULL;
			hash_q->sub[p[i]]->sub=NULL;
		}
		hash_q=hash_q->sub[p[i]];
	}
	if(hash_q->v==0){
		state_t->n=(struct m_state_st *)FMALLOC(sizeof(struct m_state_st));
		state_t=state_t->n;
		STATE_INIT(state_t);
		state_t->status=SUBROUTINE;
		state_t->beg=0;
		state_t->end=256+1;
		state_t->base=calloc(257, sizeof(struct m_state_st *));
		for(i=0;i<=256;++i){
			state_t->base[i]=state_t;
		}
		hash_q->v=(uintptr_t) state_t;
	}
	return hash_q->v;
}

int main(int argc, char *argv[]){
	int i, j, k, c=0, cu, cl, ci,pr;
	FILE *fp;
	char inbuf[1024], *f, *t, *tmp, *of, *ot;
	int dat[1024];
	offset_t sub[257];
	uintptr_t l,ret,callback_state=0;
	struct m_data_st *data_r=NULL, *data_p=NULL, *data_q=NULL, *data_t=NULL;
	struct list *todo=NULL, *newtodo, *newtodo_tail, *state_p;
	struct state_st dstate;
	struct data_st ddata;
	struct dhash *hash_p;
	int callback=0;
	void *tofree;
	char *bsdconv_mktable_fmalloc_template;

	table['0']=0;
	table['1']=1;
	table['2']=2;
	table['3']=3;
	table['4']=4;
	table['5']=5;
	table['6']=6;
	table['7']=7;
	table['8']=8;
	table['9']=9;
	table['a']=10;
	table['A']=10;
	table['b']=11;
	table['B']=11;
	table['c']=12;
	table['C']=12;
	table['d']=13;
	table['D']=13;
	table['e']=14;
	table['E']=14;
	table['f']=15;
	table['F']=15;
	ci_table['A']='a';
	ci_table['B']='b';
	ci_table['C']='c';
	ci_table['D']='d';
	ci_table['E']='e';
	ci_table['F']='f';
	ci_table['G']='g';
	ci_table['H']='h';
	ci_table['I']='i';
	ci_table['J']='j';
	ci_table['K']='k';
	ci_table['L']='l';
	ci_table['M']='m';
	ci_table['N']='n';
	ci_table['O']='o';
	ci_table['P']='p';
	ci_table['Q']='q';
	ci_table['R']='r';
	ci_table['S']='s';
	ci_table['T']='t';
	ci_table['U']='u';
	ci_table['V']='v';
	ci_table['W']='w';
	ci_table['X']='x';
	ci_table['Y']='y';
	ci_table['Z']='z';
	ci_table['a']='A';
	ci_table['b']='B';
	ci_table['c']='C';
	ci_table['d']='D';
	ci_table['e']='E';
	ci_table['f']='F';
	ci_table['g']='G';
	ci_table['h']='H';
	ci_table['i']='I';
	ci_table['j']='J';
	ci_table['k']='K';
	ci_table['l']='L';
	ci_table['m']='M';
	ci_table['n']='N';
	ci_table['o']='O';
	ci_table['p']='P';
	ci_table['q']='Q';
	ci_table['r']='R';
	ci_table['s']='S';
	ci_table['t']='T';
	ci_table['u']='U';
	ci_table['v']='V';
	ci_table['w']='W';
	ci_table['x']='X';
	ci_table['y']='Y';
	ci_table['z']='Z';

	printf("Making table %s\n", argv[1]);

#ifdef USE_FMALLOC
	bsdconv_mktable_fmalloc_template=malloc(strlen(argv[2])+10);
	sprintf(bsdconv_mktable_fmalloc_template, "%s.XXXXX", argv[2]);
	fmalloc_template=bsdconv_mktable_fmalloc_template;
#endif

	fp=fopen(argv[1], "r");

	hash_datalist=FMALLOC(sizeof(struct dhash));
	hash_datalist->sub=NULL;
	hash_datalist->p=0;
	hash_datalist->v=0;
	hash_datalist->offset=0;
	hash_datalist->head=NULL;

	hash_data=FMALLOC(sizeof(struct dhash));
	hash_data->sub=NULL;
	hash_data->p=0;
	hash_data->v=0;
	hash_data->offset=0;
	hash_data->head=NULL;

	callback_hash=FMALLOC(sizeof(struct dhash));
	callback_hash->sub=NULL;
	callback_hash->p=0;
	callback_hash->v=0;
	callback_hash->offset=0;
	callback_hash->head=NULL;

	newtodo=malloc(sizeof(struct list));
	newtodo->n=NULL;
	newtodo->l=3;
	newtodo->u=2;
	newtodo_tail=newtodo;

	state_t=state_r=(struct m_state_st *)FMALLOC(sizeof(struct m_state_st));
	STATE_INIT(state_t);
	state_t->status=DEADEND;

	holder.beg=0;
	holder.end=1;
	holder.base=calloc(257, sizeof(struct m_state_st *));
	holder.base[0]=state_r;

	while(fgets((char *)inbuf, 1024, fp)){
		if(inbuf[0]=='#') continue;
		tmp=inbuf;
		f=of=strsep((char **)&tmp, "\t ");
		while(index("\t ",*tmp)){
			++tmp;
		}
		if(*tmp){
			t=ot=strsep((char **)&tmp, "\t\r\n# ");
		}else{
			t=ot=NULL;
		}

		todo=malloc(sizeof(struct list));
		todo->n=NULL;
		todo->p=&holder;
		todo->u=0;
		todo->l=0;
		todo->pr=0;

		//read input sequence, build tree (not including leaves), generate todo list
		if(*f=='!'){
			ci=1;
			++f;
		}else{
			ci=0;
		}
		while(*f){
			if(*f=='*'){
				cl=0;
				cu=256;
			}else if(*f==','){
				cu=cl=256;
			}else{
				cl=table[(unsigned char)*f];
				++f;
				cl*=16;
				cl+=table[(unsigned char)*f];
				cu=cl;
			}
			state_p=todo;
			while(todo){
				state_p=todo;
				for(c=0;c<=256;++c){
					if(c>=state_p->l && c<=state_p->u){
						pr=1;
					}else if(ci && ci_table[c] && ci_table[c]>=state_p->l && ci_table[c]<=state_p->u){
						pr=0;
					}else{
						continue;
					}
					if(c < state_p->p->beg)
						state_p->p->beg=c;
					if(c >= state_p->p->end)
						state_p->p->end=c+1;
					if(state_p->p->base==NULL)
						state_p->p->base=calloc(257, sizeof(struct m_state_st *));
					if(state_p->p->base[c]){
						//XXX
						if(state_p->p->base[c]->status==MATCH){
							state_p->p->base[c]->status=SUBMATCH;
						}else if(state_p->p->base[c]->status==SUBROUTINE){
							state_p->p->base[c]=state_t->n=FMALLOC(sizeof(struct m_state_st));
							state_t=state_t->n;
							STATE_INIT(state_t);
						}
						newtodo_tail->n=malloc(sizeof(struct list));
						newtodo_tail=newtodo_tail->n;
						newtodo_tail->p=state_p->p->base[c];
						newtodo_tail->l=cl;
						newtodo_tail->u=cu;
						newtodo_tail->pr=state_p->pr+pr;
						newtodo_tail->n=NULL;
					}else{
						//printf("%u[%X]=%u\n", state_p->p, c, offset);
						state_t->n=state_p->p->base[c]=(struct m_state_st *)FMALLOC(sizeof(struct m_state_st));
						state_t=state_t->n;
						STATE_INIT(state_t);

						newtodo_tail->n=malloc(sizeof(struct list));
						newtodo_tail=newtodo_tail->n;
						newtodo_tail->n=NULL;
						newtodo_tail->p=state_p->p->base[c];
						newtodo_tail->l=cl;
						newtodo_tail->u=cu;
						newtodo_tail->pr=state_p->pr+pr;

						newtodo_tail->p->status=CONTINUE;
						newtodo_tail->p->data=0;
					}
				}
				todo=todo->n;
				free(state_p);
			}
			todo=newtodo->n;
			newtodo->n=NULL;
			newtodo_tail=newtodo;
			++f;
		}

		//process todo list, associate leaves with output sequence's hash id
		j=0;
		l=0;
		k=1;
		callback=0;
		if(*t=='?'){
			t+=1;
			callback=1;
		}
		while(1){
			if(*t==0){
				ret=hash(dat,l);
				if(callback)
					callback_state=hash_callback(dat, l);
				if(k){
					k=0;
					while(todo){
						state_p=todo;
						for(c=0;c<=256;++c){
							if(c>=state_p->l && c<=state_p->u){
								pr=1;
							}else if(ci && ci_table[c] && ci_table[c]>=state_p->l && ci_table[c]<=state_p->u){
								pr=0;
							}else{
								continue;
									}
							if(c < state_p->p->beg)
								state_p->p->beg=c;
							if(c >= state_p->p->end)
								state_p->p->end=c+1;
							if(state_p->p->base==NULL)
								state_p->p->base=calloc(257, sizeof(struct m_state_st *));
							if(state_p->p->base[c]){
								//XXX
								if((state_p->p->base[c]->status==MATCH || state_p->p->base[c]->status==SUBMATCH) && (pr+state_p->pr) <= state_p->p->base[c]->prio){
//									printf("Duplicated key: %s dropping data: %s\n", of, ot);
									continue;
								}else{
									if(state_p->p->base[c]->status==SUBROUTINE){
										state_p->p->base[c]=state_t->n=FMALLOC(sizeof(struct m_state_st));
										state_t=state_t->n;
										STATE_INIT(state_t);
									}
									state_p->p->base[c]->status=SUBMATCH;
								}
							}else if(callback){
								state_p->p->base[c]=(struct m_state_st *) callback_state;
							}else{
								state_p->p->base[c]=state_t->n=FMALLOC(sizeof(struct m_state_st));
								state_t=state_t->n;
								STATE_INIT(state_t);

								state_t->status=MATCH;
							}
							if(l){
								state_p->p->base[c]->data=(struct m_data_st *)ret;
							}else{
								state_p->p->base[c]->data=0;
							}
							state_p->p->base[c]->prio=pr+state_p->pr;
						}
						todo=todo->n;
						free(state_p);
					}
				}
				l=0;
				if(*t==0){
					data_p=NULL;
					break;
				}
			}else if(*t==','){
				dat[l]=256;
				++l;
			}else if(j==0){
				c=table[(unsigned char)*t];
				j=1;
			}else{
				c*=16;
				c+=table[(unsigned char)*t];
				j=0;
				dat[l]=c;
				++l;
			}
			++t;
		}
		todo=NULL;
	}
	free(newtodo);
	fclose(fp);
	free(holder.base);

	//Allocating
	state_t=state_r;
	while(state_t){
		state_t->offset=offset;
		offset+=sizeof(struct state_st);
		state_t=state_t->n;
	}

	//Write
	fopen(argv[2], "wb+");
	state_t=state_r;
	while(state_t){
		dstate.status=state_t->status;
		hash_p=(struct dhash *)state_t->data;
		k=1;	//begin of data cell
		l=0;	//length counter
		j=0;	//has read something to proceed
		data_q=NULL;	//pointer of last data cell
		while(hash_p && hash_p->p){
			if(k){
				k=0;
				if(!hash_p->offset){
					j=1;
					if(data_p){
						//data after head
						data_t->n=(struct m_data_st *)FMALLOC(sizeof(struct m_data_st));
						data_p=data_t=data_t->n;
					}else if(data_t){
						//data head
						data_t->n=(struct m_data_st *)FMALLOC(sizeof(struct m_data_st));
						data_p=data_t=data_t->n;
					}else{
						//frist
						data_t=data_p=data_r=(struct m_data_st *)FMALLOC(sizeof(struct m_data_st));
					}

					//init new cell
					hash_p->head=data_p;
					hash_p->offset=data_p->offset=offset;
					data_p->next=0;
					data_p->n=NULL;
					offset+=sizeof(struct data_st);

					data_p->data=(char *)hash_p->v;
				}
				if(data_q){
					data_q->next=hash_p->offset;
					//printf("Appending %p->%p\n",(void *)(uintptr_t)data_q->offset, (void *)(uintptr_t)data_q->next);
				}
				data_q=hash_p->head;
			}
			if(hash_p->c==256){
				k=1;
				if(j){
					j=0;
					data_p->len=l;
				}
				l=0;
			}else{
				l+=1;				
			}
			hash_p=hash_p->p;
		}
		if(j){
			j=0;
			data_p->len=l;
		}
		hash_p=(struct dhash *)state_t->data;
		if(hash_p)
			dstate.data=(struct data_st *)(uintptr_t)hash_p->offset;
		else
			dstate.data=NULL;

		dstate.beg=state_t->beg;
		dstate.end=state_t->end;
		if(state_t->base){
			dstate.base=offset;
			fseek(fp, offset, SEEK_SET);
			for(i=state_t->beg;i<state_t->end;++i)
				if(state_t->base[i])
					sub[i]=state_t->base[i]->offset;
				else
					sub[i]=0;
			fwrite(&sub[state_t->beg], sizeof(offset_t), state_t->end - state_t->beg, fp);
			offset+=(state_t->end - state_t->beg) * sizeof(offset_t);
		}else{
			dstate.base=0;
		}
		
		fseek(fp, state_t->offset, SEEK_SET);
		//printf("Writing struct state_st.\n");
		fwrite((void *)&dstate, sizeof(struct state_st), 1, fp);
		if(state_t->base)
			free(state_t->base);
		tofree=state_t;
		state_t=state_t->n;
		FFREE(tofree);
	}

	data_t=data_r;
	while(data_t){
		hash_p=(struct dhash *)data_t->data;
		if(hash_p && !hash_p->head){
			hash_p->head=data_t->data;
			hash_p=hash_p->p;
			while(hash_p && hash_p->p){
				hash_p->head=data_t->data;
				hash_p=hash_p->p;
			}
		}
		data_t=data_t->n;
	}
	
	data_t=data_r;
	while(data_t){
		hash_p=(struct dhash *)data_t->data;
		if(hash_p && !hash_p->offset){
			fseek(fp, offset, SEEK_SET);
			hash_p=hash_p->head;
			while(hash_p && hash_p->p){
				hash_p->offset=offset;
				//printf("Writing byte: %X.\n", hash_p->c);
				fputc(hash_p->c, fp);
				offset+=1;
				hash_p=hash_p->p;
			}
		}
		hash_p=(struct dhash *)data_t->data;
		if(hash_p)
			ddata.data=(char *)(uintptr_t)hash_p->offset;
		else
			ddata.data=NULL;
		ddata.len=data_t->len;
		ddata.next=(struct data_st *)(uintptr_t)data_t->next;
		fseek(fp, data_t->offset, SEEK_SET);
		//printf("Writing struct data_st at %p %c %p.\n", (void *)(uintptr_t)data_t->offset, hash_p->p->c, ddata.next);
		fwrite((void *)&ddata, sizeof(struct data_st), 1, fp);
		tofree=data_t;
		data_t=data_t->n;
		FFREE(tofree);
	}
	fclose(fp);
	printf("Total size: %u\n", (unsigned int)offset);

#ifdef USE_FMALLOC
	free(bsdconv_mktable_fmalloc_template);
	fmcleanup();
#endif

	return 0;
}
