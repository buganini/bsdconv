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

	struct m_data_s *p;
	unsigned char *dp;
	struct m_data_s *n;

};

struct m_state_s{
	char status;
	struct state_s *sub[257];

	struct m_data_s *data;

	struct m_state_s *psub[257];
	struct m_state_s *p;
	struct m_state_s *n;
	int child;
};

unsigned char table[256]={};

int offset=0;

int main(int argc, char *argv[]){
	int i, j, k, l, c;
	FILE *fp;
	unsigned char inbuf[1024], *f, *t, dat[256], *tmp, *of, *ot;
	struct m_data_s *data_r=NULL, *data_p=NULL, *data_t=NULL;
	struct m_state_s *state_r, *state_p, *state_t;
	struct state_s dstate;
	struct data_s ddata;
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

	DPRINTF("STATUS CONTINUE: %d", CONTINUE);
	DPRINTF("STATUS DEADEND: %d", DEADEND);
	DPRINTF("STATUS MATCH: %d", MATCH);
	DPRINTF("STATUS SUBMATCH: %d", SUBMATCH);
	state_t=state_p=state_r=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_p->status=DEADEND;
	state_p->data=0;
	state_p->p=(struct m_state_s *)offset;
	state_t->n=NULL;
	offset+=sizeof(struct state_s);
	for(i=0;i<257;i++){
		state_r->sub[i]=0;
		state_r->psub[i]=NULL;
	}

	while(fgets((char *)inbuf, 1024, fp)){
		if(inbuf[0]=='#') continue;
		tmp=inbuf;
		f=of=(unsigned char *)strsep((char **)&tmp, "\t ");
		t=ot=(unsigned char *)strsep((char **)&tmp, "\t\r\n# ");
		state_p=state_r;
		while(*f){
			if(*f==','){
				c=256;
			}else{
				c=table[*f];
				++f;
				c*=16;
				c+=table[*f];
			}
			if(state_p->psub[c]){
				if(state_p->status==MATCH){
					state_p->status=SUBMATCH;
				}
				state_p=state_p->psub[c];
			}else{
//				printf("%X[%X]=%X\n", (int)state_p->p, (int)c, (int)offset);
				state_p->psub[c]=(struct m_state_s *)malloc(sizeof(struct m_state_s));
				state_p->sub[c]=(struct state_s *)offset;
				state_p->child++;

				state_p=state_t->n=state_p->psub[c];

				state_t=state_t->n;
				state_t->n=NULL;

				state_p->p=(struct m_state_s *)offset;
				offset+=sizeof(struct state_s);
				state_p->status=CONTINUE;
				state_p->data=0;
				state_p->child=0;
				for(i=0;i<257;i++){
					state_p->sub[i]=0;
					state_p->psub[i]=NULL;
				}
			}
			++f;
		}

		if(state_p->data){
			printf("Duplicated key: %s, dropping data: %s\n", of, ot);
			continue;
		}

		j=0;
		l=0;
		k=1;
		while(1){
			if(*t==',' || *t==0){
				if(l){
					if(data_p){
						//make new cell
						data_t->n=(struct m_data_s *)malloc(sizeof(struct m_data_s));
						data_p->next=(struct data_s *)offset;
						data_p=data_t=data_t->n;
DPRINTF("%d.next=%d", (int)data_p->p, offset);
					}else if(data_t){
						//make new cell
						data_t->n=(struct m_data_s *)malloc(sizeof(struct m_data_s));
						data_p=data_t=data_t->n;
					}else{
						//make new cell
						data_t=data_p=data_r=(struct m_data_s *)malloc(sizeof(struct m_data_s));
					}

					//init new cell
					data_p->p=(struct m_data_s *)offset;
					data_p->next=0;
					data_p->n=NULL;
					offset+=sizeof(struct data_s);

					//put data
					data_p->dp=(unsigned char *)malloc(l);
					memcpy(data_p->dp, dat, l);
					data_p->len=l;
					data_p->data=(unsigned char *)offset;
					offset+=l;

					DPRINTF("%d.data=(%d) %d", (int)data_p->p, data_p->len, (int)data_p->data);
					if(k){
						k=0;
						if(state_p->child){
							state_p->status=SUBMATCH;
						}else{
							state_p->status=MATCH;
						}
						state_p->data=data_p->p;
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
		memcpy(&tmp[(int)state_t->p], &dstate, sizeof(struct state_s));
		tofree=state_t;
		state_t=state_t->n;
		free(tofree);
	}
	data_t=data_r;
	while(data_t){
		ddata.data=data_t->data;
		ddata.len=data_t->len;
		ddata.next=data_t->next;
		memcpy(&tmp[(int)data_t->p], &ddata, sizeof(struct data_s));
		memcpy(&tmp[(int)ddata.data], data_t->dp, ddata.len);
		tofree=data_t;
		data_t=data_t->n;
		free(data_t);
	}
	munmap(tmp,offset);
	close(k);
	return 0;
}
