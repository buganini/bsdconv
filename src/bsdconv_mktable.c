#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "bsdconv.h"

struct m_data_s{
	int data;
	size_t len;
	int next;
	int p;
	char *dp;
	struct m_data_s *n;
};

struct m_state_s{
	int status;
	int data;
	int sub[257];
	struct m_state_s *psub[257];
	int p;
	struct m_state_s *n;
};

unsigned char table[256]={};

int offset=0;

int main(int argc, char *argv[]){
	int i, j, k, l, c;
	int state_deadend;
	FILE *in;
	char f[256], t[256], dat[256], *tmp;
	struct m_data_s *data_r, *data_p=NULL, *data_t=NULL;
	struct m_state_s *state_r, *state_p, *state_t;
	struct stat stat;

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
	
	in=fopen(argv[1], "r");

	state_t=state_p=state_r=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_p->status=DEADEND;
	state_p->data=0;
	state_p->p=offset;
	state_t->n=NULL;
	offset+=sizeof(struct state_s);
	for(i=0;i<257;i++){
		state_r->sub[i]=0;
	}

	while(fscanf(in, "%s\t%s\n", f, t)==2){
		state_p=state_r;
		tmp=f;
		j=1;
		while(*tmp){
			if(*tmp==','){
				j=1;
				c=257;
			}else if(j){
				c=table[*tmp];
				j=0;
			}else{
				c*=16;
				c+=table[*tmp];
				j=1;
			}
			if(j){
				if(state_p->status==DEADEND){
					state_p->status=CONTINUE;
				}
				if(!state_p->psub[c]){
					state_p->psub[c]=(struct m_state_s *)malloc(sizeof(struct m_state_s));
					state_p->sub[c]=offset;
					state_t->n=state_p->psub[c];
					state_t=state_t->n;
					state_t->n=NULL;
					state_p=state_p->psub[c];
					state_p->p=offset;
					offset+=sizeof(struct state_s);
					state_p->status=DEADEND;
					state_p->data=0;
					for(i=0;i<257;i++){
						state_p->sub[i]=0;
						state_p->psub[i]=NULL;
					}
				}else{
					state_p=state_p->psub[c];
				}
			}
			++tmp;
		}
		tmp=t;
		j=1;
		l=0;
		k=1;
		while(*tmp){
			if(*tmp==','){
				flush:
				if(l){
					if(data_p){
						//make new cell
						data_t->n=(struct m_data_s *)malloc(sizeof(struct m_data_s));
						data_p->next=offset;
						data_p=data_t=data_t->n;

						//init new cell
						data_p->p=offset;
						data_p->next=0;
						data_p->n=NULL;
						offset+=sizeof(struct data_s);

						//put data
						data_p->dp=(char *)malloc(l);
						memcpy(data_p->dp,dat,l);
						data_p->len=l;
						data_p->data=offset;
						offset+=l;
					}else if(data_t){
						//make new cell
						data_t->n=(struct m_data_s *)malloc(sizeof(struct m_data_s));
						data_p=data_t=data_t->n;

						//init new cell
						data_p->p=offset;
						data_p->next=0;
						data_p->n=NULL;
						offset+=sizeof(struct data_s);

						//put data
						data_p->dp=(char *)malloc(l);
						memcpy(data_p->dp,dat,l);
						data_p->len=l;
						data_p->data=offset;
						offset+=l;
					}else{
						//make new cell
						data_t=data_p=data_r=(struct m_data_s *)malloc(sizeof(struct m_data_s));

						//init new cell
						data_p->p=offset;
						data_p->next=0;
						data_p->n=NULL;
						offset+=sizeof(struct data_s);

						//put data
						data_p->dp=(char *)malloc(l);
						memcpy(data_p->dp, dat, l);
						data_p->len=l;
						data_p->data=offset;
						offset+=l;
					}
					if(k){
						k=0;
						state_p->status=MATCH;
						state_p->data=data_p->p;
					}
					l=0;
				}
			}else{
				if(j){
					c=table[*tmp];
					j=0;
				}else{
					c*=16;
					c+=table[*tmp];
					j=1;
					dat[l]=c;
					++l;
				}
			}
			++tmp;
		}
		goto flush;
		data_p=NULL;
	}
}
