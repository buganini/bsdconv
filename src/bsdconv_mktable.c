#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "bsdconv.h"

struct m_data_s{
	unsigned char *data;
	size_t len;
	struct data_s *next;

	unsigned int p;
	unsigned char *dp;
	struct m_data_s *n;
};

struct m_state_s{
	char status;
	struct state_s *sub[257];

	unsigned int data;

	struct m_state_s *psub[257];
	unsigned int p;
	struct m_state_s *n;
	int child;
};

struct list{
	struct m_state_s *p;
	int u,l;
	struct list *n;
};

unsigned char table[256]={};

int offset=0;

int main(int argc, char *argv[]){
	int i, j, k, l, c, cu, cl;
	FILE *fp;
	unsigned char inbuf[1024], *f, *t, dat[256], *tmp, *of, *ot;
	struct m_data_s *data_r=NULL, *data_p=NULL, *data_t=NULL;
	struct m_state_s *state_r, *state_t, holder, *state_i;
	struct list *todo=NULL, *newtodo, *newtodo_tail, *state_p;
	struct state_s dstate;
	struct data_s ddata;
	int callback=0;
	void *tofree;

	printf("Making table %s\n", argv[1]);

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
	fp=fopen(argv[1], "r");

	newtodo=malloc(sizeof(struct list));
	newtodo_tail=newtodo;

	DPRINTF("STATUS CONTINUE: %d", CONTINUE);
	DPRINTF("STATUS DEADEND: %d", DEADEND);
	DPRINTF("STATUS MATCH: %d", MATCH);
	DPRINTF("STATUS SUBMATCH: %d", SUBMATCH);
	state_t=state_r=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_t->status=DEADEND;
	state_t->data=0;
	state_t->p=offset;
	state_t->n=NULL;
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
		t=ot=(unsigned char *)strsep((char **)&tmp, "\t\r\n# ");

		todo=malloc(sizeof(struct list));
		todo->n=NULL;
		todo->p=&holder;
		todo->u=0;
		todo->l=0;

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
				for(c=state_p->l;c<=state_p->u;c++){
					if(state_p->p->psub[c]){
						if(state_p->p->status==MATCH){
							state_p->p->status=SUBMATCH;
						}
						newtodo_tail->n=malloc(sizeof(struct list));
						newtodo_tail=newtodo_tail->n;
						newtodo_tail->p=state_p->p->psub[c];
						newtodo_tail->l=cl;
						newtodo_tail->u=cu;
						newtodo_tail->n=NULL;
					}else{
		//	printf("%u[%X]=%u\n", state_p->p, c, offset);
						state_p->p->psub[c]=(struct m_state_s *)malloc(sizeof(struct m_state_s));
						state_p->p->sub[c]=(struct state_s *)offset;
						state_p->p->child++;
						state_p->p->psub[c]->p=offset;
						offset+=sizeof(struct state_s);

						newtodo_tail->n=malloc(sizeof(struct list));
						newtodo_tail=newtodo_tail->n;
						newtodo_tail->n=NULL;
						newtodo_tail->p=state_p->p->psub[c];
						newtodo_tail->l=cl;
						newtodo_tail->u=cu;

						state_t->n=state_p->p->psub[c];
						state_t=state_t->n;
						state_t->n=NULL;

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
				state_t->status=CALLBACK;
				state_t->data=0;
				state_t->p=offset;
				state_t->n=NULL;
				callback=offset;
				for(i=0;i<256;i++){
					state_t->sub[i]=(struct state_s *)callback;
					state_t->psub[i]=NULL;
				}
				state_t->sub[256]=0;
				offset+=sizeof(struct state_s);
			}
			while(todo){
				state_p=todo;
				for(c=state_p->l;c<=state_p->u;c++){
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

					DPRINTF("%u.data=(%d) %d", data_p->p, data_p->len, (int)data_p->data);
					if(k){
						k=0;
						while(todo){
							state_p=todo;
							for(c=state_p->l;c<=state_p->u;c++){
								if(state_p->p->psub[c]){
									if(state_p->p->psub[c]->data){
										printf("Duplicated key: %s dropping data: %s\n", of, ot);
										continue;
									}else{
										state_p->p->psub[c]->status=SUBMATCH;
									}
								}else{
									state_p->p->psub[c]=state_t->n=malloc(sizeof(struct m_state_s));
									state_p->p->sub[c]=offset;
									state_t=state_t->n;
									state_t->p=offset;
									state_t->status=MATCH;
									state_t->data=data_p->p;
									offset+=sizeof(struct state_s);
								}
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
	k=open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0644);
	ftruncate(k,offset);
	printf("Total size: %d\n", offset);
	tmp=mmap(0,offset,PROT_READ|PROT_WRITE,MAP_SHARED,k,0);
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
		memcpy(&tmp[(int)ddata.data], data_t->dp, ddata.len);
		tofree=data_t;
		data_t=data_t->n;
		free(data_t);
	}
	munmap(tmp,offset);
	close(k);
	return 0;
}
