#define DEBUG
#ifdef DEBUG
#define DPRINTF(fmt, args...) printf("DEBUG: " fmt "\n", ## args); fflush(stdout);
#else
#define DPRINTF(fmt, args...)
#endif

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
	int i, j, k, l, c, deadend, flushing;
	FILE *fp;
	char inbuf[1024], *f, *t, dat[256], *tmp;
	struct m_data_s *data_r, *data_p=NULL, *data_t=NULL;
	struct m_state_s *state_r, *state_p, *state_t;
	struct state_s dstate;
	struct data_s ddata;
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
	
	fp=fopen(argv[1], "r");

	state_t=state_p=state_r=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_p->status=DEADEND;
	state_p->data=0;
	state_p->p=offset;
	state_t->n=NULL;
	offset+=sizeof(struct state_s);

	state_t->n=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_t->status=DEADEND;
	state_t->data=0;
	deadend=state_t->p=offset;
	state_t=state_t->n;
	state_t->n=NULL;
	offset+=sizeof(struct state_s);

	for(i=0;i<257;i++){
		state_r->sub[i]=deadend;
		state_r->psub[i]=NULL;
		state_t->sub[i]=deadend;
		state_t->psub[i]=NULL;
	}

	while(fgets(inbuf, 1024, fp)){
		if(inbuf[0]=='#') continue;
		tmp=inbuf;
		f=strsep(&tmp, "\t ");
		t=strsep(&tmp, "\t\r\n# ");
		state_p=state_r;
		j=1;
		while(*f){
			if(*f==','){
				j=1;
				c=256;
			}else if(j){
				c=table[*f];
				j=0;
			}else{
				c*=16;
				c+=table[*f];
				j=1;
			}
			if(j){
				if(state_p->status==DEADEND){
					state_p->status=CONTINUE;
				}
				if(state_p->psub[c]){
					state_p=state_p->psub[c];
				}else{
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
						state_p->sub[i]=deadend;
						state_p->psub[i]=NULL;
					}
				}
			}
			++f;
		}
		j=1;
		l=0;
		k=1;
		flushing=0;
		while(*t){
			if(*t==','){
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
					if(flushing){
						break;
					}
					l=0;
				}
			}else{
				if(j){
					c=table[*t];
					j=0;
				}else{
					c*=16;
					c+=table[*t];
					j=1;
					dat[l]=c;
					++l;
				}
			}
			++t;
		}
		if(flushing){
			data_p=NULL;
		}else{
			flushing=1;
			goto flush;
		}
	}
	fclose(fp);
	k=open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0644);
	ftruncate(k,offset);
	printf("Total size: %d\n", offset);
	tmp=mmap(0,offset,PROT_READ|PROT_WRITE,MAP_SHARED,k,0);
	state_t=state_r;
	while(state_t){
		dstate.status=state_t->status;
		dstate.data=state_t->data;
		memcpy(dstate.sub, state_t->sub, 257);
		memcpy(&tmp[state_t->p], &dstate, sizeof(struct state_s));
		state_t=state_t->n;
	}
	data_t=data_r;
	while(data_t){
		ddata.data=data_t->data;
		ddata.len=data_t->len;
		ddata.next=data_t->next;
		ddata.p=data_t->p;
		memcpy(&tmp[data_t->p], &ddata, sizeof(struct data_s));
		memcpy(&tmp[ddata.data], data_t->dp, ddata.len);
		data_t=data_t->n;
	}
	munmap(tmp,offset);
	close(k);
	return 0;
}
