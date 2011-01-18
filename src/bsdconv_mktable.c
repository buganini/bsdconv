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
#ifndef WIN32
#include <errno.h>
#endif


struct m_data_st{
	char *data;
	size_t len;
	struct data_st *next;

	uintptr_t p;
	struct m_data_st *n;
};

struct m_state_st{
	char status;
	struct state_st *sub[257];

	uintptr_t data;

	int prio;

	struct m_state_st *psub[257];
	uintptr_t p;
	struct m_state_st *n;
};

struct list{
	struct m_state_st *p;
	int u,l,pr;
	struct list *n;
};

struct dhash{
	uintptr_t v;
	int c;
	struct dhash *p;
	struct dhash *sub[257];
	uintptr_t offset;
	uintptr_t head;
};

char table[256]={};
char ci_table[256]={0};

uintptr_t offset=0;

struct dhash *hash_datalist, *hash_data;

uintptr_t hash(int *p, uintptr_t l){
	int i,j;
	struct dhash *hash_p=hash_datalist;
	struct dhash *hash_q=hash_data;
	for(i=l-1;i>=0;--i){
		if(hash_p->sub[p[i]]==NULL){
			hash_p->sub[p[i]]=malloc(sizeof(struct dhash));
			hash_p->sub[p[i]]->c=p[i];
			hash_p->sub[p[i]]->p=hash_p;
			hash_p->sub[p[i]]->v=0;
			hash_p->sub[p[i]]->offset=0;
			for(j=0;j<=256;++j){
				hash_p->sub[p[i]]->sub[j]=NULL;
			}
		}
//		printf("hash_p: [%d]%X %p => %p\n", i, p[i], hash_p, hash_p->sub[p[i]]);
		if(p[i]==256){
			hash_p->v=(uintptr_t) hash_q;
			hash_p=hash_p->sub[p[i]];
			hash_q=hash_data;
		}else{
			hash_p=hash_p->sub[p[i]];
			if(hash_q->sub[p[i]]==NULL){
				hash_q->sub[p[i]]=malloc(sizeof(struct dhash));
				hash_q->sub[p[i]]->c=p[i];
				hash_q->sub[p[i]]->p=hash_q;
				hash_q->sub[p[i]]->v=0;
				hash_q->sub[p[i]]->offset=0;
				hash_q->sub[p[i]]->head=0;
				for(j=0;j<=256;++j){
					hash_q->sub[p[i]]->sub[j]=NULL;
				}
			}
			hash_q=hash_q->sub[p[i]];
		}
	}
	hash_p->v=(uintptr_t) hash_q;
	return (uintptr_t) hash_p;
}

int main(int argc, char *argv[]){
	int i, j, k, c=0, cu, cl, ci,pr;
	FILE *fp;
	char inbuf[1024], *f, *t, *tmp, *of, *ot;
	int dat[1024];
	uintptr_t l,ret;
	struct m_data_st *data_r=NULL, *data_p=NULL, *data_t=NULL;
	struct m_state_st *state_r, *state_t, holder;
	struct list *todo=NULL, *newtodo, *newtodo_tail, *state_p;
	struct state_st dstate;
	struct data_st ddata;
	struct dhash *hash_p;
	uintptr_t callback=0;
	void *tofree;

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

	fp=fopen(argv[1], "r");

	hash_datalist=malloc(sizeof(struct dhash));
	for(i=0;i<=256;++i){
		hash_datalist->sub[i]=0;
	}
	hash_datalist->p=0;
	hash_datalist->v=0;
	hash_datalist->offset=0;
	hash_datalist->head=0;

	hash_data=malloc(sizeof(struct dhash));
	for(i=0;i<=256;++i){
		hash_data->sub[i]=0;
	}
	hash_data->p=0;
	hash_data->v=0;
	hash_data->offset=0;
	hash_data->head=0;

	newtodo=malloc(sizeof(struct list));
	newtodo->n=NULL;
	newtodo->l=3;
	newtodo->u=2;
	newtodo_tail=newtodo;

	state_t=state_r=(struct m_state_st *)malloc(sizeof(struct m_state_st));
	state_t->status=DEADEND;
	state_t->data=0;
	state_t->p=offset;
	state_t->n=NULL;
	offset+=sizeof(struct state_st);
	for(i=0;i<257;i++){
		state_r->sub[i]=0;
		state_r->psub[i]=NULL;
	}

	holder.psub[0]=state_r;

	while(fgets((char *)inbuf, 1024, fp)){
		if(inbuf[0]=='#') continue;
		tmp=inbuf;
		f=of=strsep((char **)&tmp, "\t ");
		while(index("\t ",*tmp)){
			tmp++;
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

		if(*f=='!'){
			ci=1;
			++f;
		}else{
			ci=0;
		}
		while(*f){
			if(*f=='*'){
				cl=0;
				cu=255;
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
				for(c=0;c<=256;c++){
					if(c>=state_p->l && c<=state_p->u){
						pr=1;
					}else if(ci && ci_table[c] && ci_table[c]>=state_p->l && ci_table[c]<=state_p->u){
						pr=0;
					}else{
						continue;
					}
					if(state_p->p->psub[c]){
						if(state_p->p->psub[c]->status==MATCH){
							state_p->p->psub[c]->status=SUBMATCH;
						}
						newtodo_tail->n=malloc(sizeof(struct list));
						newtodo_tail=newtodo_tail->n;
						newtodo_tail->p=state_p->p->psub[c];
						newtodo_tail->l=cl;
						newtodo_tail->u=cu;
						newtodo_tail->pr=state_p->pr+pr;
						newtodo_tail->n=NULL;
					}else{
		//	printf("%u[%X]=%u\n", state_p->p, c, offset);
						state_t->n=state_p->p->psub[c]=(struct m_state_st *)malloc(sizeof(struct m_state_st));
						state_t=state_t->n;
						state_t->n=NULL;
						for(i=0;i<257;i++){
							state_t->sub[i]=0;
							state_t->psub[i]=NULL;
						}
						state_p->p->sub[c]=(struct state_st *)offset;
						state_p->p->psub[c]->p=offset;
						offset+=sizeof(struct state_st);

						state_t->n=state_p->p->psub[c];
						state_t=state_t->n;
						state_t->n=NULL;

						newtodo_tail->n=malloc(sizeof(struct list));
						newtodo_tail=newtodo_tail->n;
						newtodo_tail->n=NULL;
						newtodo_tail->p=state_p->p->psub[c];
						newtodo_tail->l=cl;
						newtodo_tail->u=cu;
						newtodo_tail->pr=state_p->pr+pr;

						newtodo_tail->p->status=CONTINUE;
						newtodo_tail->p->data=0;
						for(i=0;i<=256;i++){
							newtodo_tail->p->sub[i]=0;
							newtodo_tail->p->psub[i]=NULL;
						}
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

		j=0;
		l=0;
		k=1;
		if(*t=='?'){
			if(!callback){
				state_t->n=(struct m_state_st *)malloc(sizeof(struct m_state_st));
				state_t=state_t->n;
				state_t->status=SUBROUTINE;
				state_t->data=0;
				state_t->p=offset;
				state_t->n=NULL;
				callback=offset;
				for(i=0;i<257;i++){
					state_t->sub[i]=(struct state_st *)callback;
					state_t->psub[i]=NULL;
				}
				state_t->sub[256]=0;
				offset+=sizeof(struct state_st);
			}
			while(todo){
				state_p=todo;
				for(c=0;c<=256;c++){
					if(c>=state_p->l && c<=state_p->u){
						pr=1;
					}else if(ci && ci_table[c] && ci_table[c]>=state_p->l && ci_table[c]<=state_p->u){
						pr=0;
					}else{
						continue;
					}
					state_p->p->sub[c]=(struct state_st *)callback;
				}
				todo=todo->n;
				free(state_p);
			}
		}else while(1){
			if(*t==0){
				ret=hash(dat,l);
				if(k){
					k=0;
					while(todo){
						state_p=todo;
						for(c=0;c<=256;c++){
							if(c>=state_p->l && c<=state_p->u){
								pr=1;
							}else if(ci && ci_table[c] && ci_table[c]>=state_p->l && ci_table[c]<=state_p->u){
								pr=0;
							}else{
								continue;
							}
							if(state_p->p->psub[c]){
								if(state_p->p->psub[c]->data && (pr+state_p->pr) <= state_p->p->psub[c]->prio){
//									printf("Duplicated key: %s dropping data: %s\n", of, ot);
									continue;
								}else{
									state_p->p->psub[c]->status=SUBMATCH;
								}
							}else{
								state_p->p->psub[c]=state_t->n=malloc(sizeof(struct m_state_st));
								state_p->p->sub[c]=(struct state_st *)offset;
								state_t=state_t->n;
								for(i=0;i<=256;i++){
									state_t->sub[i]=0;
									state_t->psub[i]=NULL;
								}
								state_t->n=NULL;
								state_t->p=offset;
								state_t->status=MATCH;
								offset+=sizeof(struct state_st);
							}
							if(l){
								state_p->p->psub[c]->data=ret;
							}else{
								state_p->p->psub[c]->data=0;
							}
							state_p->p->psub[c]->prio=pr+state_p->pr;
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
	fopen(argv[2], "wb+");
	state_t=state_r;
	while(state_t){
		dstate.status=state_t->status;
		hash_p=(struct dhash *)state_t->data;
		k=1;	//begin of data cell
		l=0;	//length counter
		j=0;	//has read something to proceed
		while(hash_p && hash_p->p){
			if(k){
				k=0;
				if(!hash_p->offset){
					j=1;
					if(data_p){
						//data after head
						//make new cell
						data_t->n=(struct m_data_st *)malloc(sizeof(struct m_data_st));
						data_p=data_t=data_t->n;
					}else if(data_t){
						//data head
						//make new cell
						data_t->n=(struct m_data_st *)malloc(sizeof(struct m_data_st));
						data_p=data_t=data_t->n;
					}else{
						//frist
						//make new cell
						data_t=data_p=data_r=(struct m_data_st *)malloc(sizeof(struct m_data_st));
					}

					//init new cell
					hash_p->offset=data_p->p=offset;
					data_p->next=0;
					data_p->n=NULL;
					offset+=sizeof(struct data_st);

					data_p->data=(char *)hash_p->v;
				}
			}
			if(hash_p->c==256){
				k=1;
				if(j){
					j=0;
					data_p->len=l;
					data_p->next=(struct data_st *)offset;
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
			data_p->next=0;
		}
		
		hash_p=(struct dhash *)state_t->data;
		if(hash_p)
			dstate.data=(struct data_st *)hash_p->offset;
		else
			dstate.data=NULL;
		for(i=0;i<257;i++){
			dstate.sub[i]=state_t->sub[i];
		}
		fseek(fp, state_t->p, SEEK_SET);
		//printf("Writing struct state_st.\n");
		fwrite((void *)&dstate, sizeof(struct state_st), 1, fp);
		tofree=state_t;
		state_t=state_t->n;
		free(tofree);
	}

	data_t=data_r;
	while(data_t){
		hash_p=(struct dhash *)data_t->data;
		if(hash_p && !hash_p->head){
			hash_p->head=(uintptr_t)data_t->data;
			hash_p=hash_p->p;
			while(hash_p && hash_p->p){
				hash_p->head=(uintptr_t)data_t->data;
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
			hash_p=(struct dhash *)hash_p->head;
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
			ddata.data=(char *)hash_p->offset;
		else
			ddata.data=NULL;
		ddata.len=data_t->len;
		ddata.next=data_t->next;
		fseek(fp, data_t->p, SEEK_SET);
		//printf("Writing struct data_st.\n");
		fwrite((void *)&ddata, sizeof(struct data_st), 1, fp);
		tofree=data_t;
		data_t=data_t->n;
		free(tofree);
	}
	fclose(fp);
	printf("Total size: %u\n", (unsigned int)offset);
	return 0;
}
