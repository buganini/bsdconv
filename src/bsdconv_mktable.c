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

#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/mman.h>
#endif
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


struct m_data_s{
	unsigned char *data;
	size_t len;
	struct data_s *next;

	uintptr_t p;
	unsigned char *dp;
	struct m_data_s *n;
};

struct m_state_s{
	char status;
	struct state_s *sub[257];

	uintptr_t data;

	int prio;

	struct m_state_s *psub[257];
	uintptr_t p;
	struct m_state_s *n;
	int child;
};

struct list{
	struct m_state_s *p;
	int u,l,pr;
	struct list *n;
};

unsigned char table[256]={};
unsigned char ci_table[256]={0};

uintptr_t offset=0;

int main(int argc, char *argv[]){
	int i, j, k, l, c=0, cu, cl, ci,pr;
	FILE *fp;
	unsigned char inbuf[1024], *f, *t, dat[256], *tmp, *of, *ot;
	struct m_data_s *data_r=NULL, *data_p=NULL, *data_t=NULL;
	struct m_state_s *state_r, *state_t, holder;
	struct list *todo=NULL, *newtodo, *newtodo_tail, *state_p;
	struct state_s dstate;
	struct data_s ddata;
	uintptr_t callback=0;
	void *tofree;
#ifdef WIN32
	HANDLE fd;
	HANDLE md;
#endif

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

	newtodo=malloc(sizeof(struct list));
	newtodo->n=NULL;
	newtodo->l=3;
	newtodo->u=2;
	newtodo_tail=newtodo;

	state_t=state_r=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_t->status=DEADEND;
	state_t->data=0;
	state_t->p=offset;
	state_t->n=NULL;
	state_t->child=0;
	offset+=sizeof(struct state_s);
	for(i=0;i<257;i++){
		state_r->sub[i]=0;
		state_r->psub[i]=NULL;
	}

	holder.psub[0]=state_r;

	while(fgets((char *)inbuf, 1024, fp)){
		if(inbuf[0]=='#') continue;
		tmp=inbuf;
		f=of=(unsigned char *)strsep((char **)&tmp, "\t ");
		while(index("\t ",*tmp)){
			tmp++;
		}
		if(*tmp){
			t=ot=(unsigned char *)strsep((char **)&tmp, "\t\r\n# ");
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
				cl=table[*f];
				++f;
				cl*=16;
				cl+=table[*f];
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
						state_t->n=state_p->p->psub[c]=(struct m_state_s *)malloc(sizeof(struct m_state_s));
						state_t=state_t->n;
						state_t->child=0;
						state_t->n=NULL;
						for(i=0;i<257;i++){
							state_t->sub[i]=0;
							state_t->psub[i]=NULL;
						}
						state_p->p->sub[c]=(struct state_s *)offset;
						state_p->p->child++;
						state_p->p->psub[c]->p=offset;
						offset+=sizeof(struct state_s);

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
						newtodo_tail->p->child=0;
						for(i=0;i<257;i++){
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
				state_t->n=(struct m_state_s *)malloc(sizeof(struct m_state_s));
				state_t=state_t->n;
				state_t->status=SUBROUTINE;
				state_t->data=0;
				state_t->p=offset;
				state_t->n=NULL;
				state_t->child=0;
				callback=offset;
				for(i=0;i<257;i++){
					state_t->sub[i]=(struct state_s *)callback;
					state_t->psub[i]=NULL;
				}
				state_t->sub[256]=0;
				offset+=sizeof(struct state_s);
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
					state_p->p->sub[c]=(struct state_s *)callback;
				}
				todo=todo->n;
				free(state_p);
			}
		}else while(1){
			if(*t==',' || *t==0){
				if(l){
					if(data_p){
						//data after head
						//make new cell
						data_t->n=(struct m_data_s *)malloc(sizeof(struct m_data_s));
						data_p->next=(struct data_s *)offset;
						data_p=data_t=data_t->n;
					}else if(data_t){
						//data head
						//make new cell
						data_t->n=(struct m_data_s *)malloc(sizeof(struct m_data_s));
						data_p=data_t=data_t->n;
					}else{
						//frist
						//make new cell
						data_t=data_p=data_r=(struct m_data_s *)malloc(sizeof(struct m_data_s));
					}

					//init new cell
					data_p->p=offset;
					data_p->next=0;
					data_p->n=NULL;
					offset+=sizeof(struct data_s);

					//put data
					data_p->dp=(unsigned char *)malloc(l);
					memcpy(data_p->dp, dat, l);
					data_p->len=l;
					data_p->data=(unsigned char *)offset;
					offset+=l;

					if(k){
						k=0;
						while(todo){
							state_p=todo;
							for(c=0;c<256;c++){
								if(c>=state_p->l && c<=state_p->u){
									pr=1;
								}else if(ci && ci_table[c] && ci_table[c]>=state_p->l && ci_table[c]<=state_p->u){
									pr=0;
								}else{
									continue;
								}
								if(state_p->p->psub[c]){
									if(state_p->p->psub[c]->data && (pr+state_p->pr) <= state_p->p->psub[c]->prio){
										printf("Duplicated key: %s dropping data: %s\n", of, ot);
										continue;
									}else{
										state_p->p->psub[c]->status=SUBMATCH;
									}
								}else{
									state_p->p->psub[c]=state_t->n=malloc(sizeof(struct m_state_s));
									state_p->p->sub[c]=(struct state_s *)offset;
									state_t=state_t->n;
									for(i=0;i<=256;i++){
										state_t->sub[i]=0;
										state_t->psub[i]=NULL;
									}
									state_t->n=NULL;
									state_t->child=0;
									state_t->p=offset;
									state_t->status=MATCH;
									offset+=sizeof(struct state_s);
								}
								state_p->p->psub[c]->data=data_p->p;
								state_p->p->psub[c]->prio=pr+state_p->pr;
							}
							todo=todo->n;
							free(state_p);
						}
					}
					if(*t==0){
						data_p=NULL;
						break;
					}
					l=0;
				}
			}else{
				if(j==0){
					c=table[(int)*t];
					j=1;
				}else{
					c*=16;
					c+=table[(int)*t];
					j=0;
					dat[l]=c;
					++l;
				}
			}
			++t;
		}
		todo=NULL;
	}
	fclose(fp);
	if((k=open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0644))<0){
		fprintf(stderr, "Failed open output file (%d).\n", GetLastError());
	}
	if(ftruncate(k,offset)<0){
		fprintf(stderr, "Failed ftruncating (%d).\n", GetLastError());
	}
#ifdef WIN32
	close(k);
	fd=CreateFile(argv[2], GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	md=CreateFileMapping(fd, NULL, PAGE_READWRITE, 0,0, NULL);
	tmp=MapViewOfFile(md, FILE_MAP_READ|FILE_MAP_WRITE, 0,0,0);
#else
	tmp=mmap(0,offset,PROT_READ|PROT_WRITE,MAP_SHARED,k,0);
#endif
	printf("Total size: %u\n", (unsigned int)offset);
	state_t=state_r;
	while(state_t){
		dstate.status=state_t->status;
		dstate.data=(struct data_s *)state_t->data;
		for(i=0;i<257;i++){
			dstate.sub[i]=state_t->sub[i];
		}
		memcpy(&tmp[state_t->p], &dstate, sizeof(struct state_s));
		tofree=state_t;
		state_t=state_t->n;
		free(tofree);
	}
	data_t=data_r;
	while(data_t){
		ddata.data=data_t->data;
		ddata.len=data_t->len;
		ddata.next=data_t->next;
		memcpy(&tmp[data_t->p], &ddata, sizeof(struct data_s));
		memcpy(&tmp[(uintptr_t)ddata.data], data_t->dp, ddata.len);
		tofree=data_t;
		data_t=data_t->n;
		free(tofree);
	}
#ifdef WIN32
	UnmapViewOfFile(tmp);
	CloseHandle(md);
	CloseHandle(fd);
#else
	munmap(tmp,offset);
	close(k);
#endif
	return 0;
}
