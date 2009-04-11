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
};

struct m_state_s{
	int status;
	int data;
	int sub[257];
	int p;
};

unsigned char table[256]={};

int offset=0;

int main(int argc, char *argv[]){
	int i, j;
	FILE *in;
	char f[256], t[256], *tmp, c;
	struct m_data_s *data_r, *data_p;
	struct m_state_s *state_r, *state_p;
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

	state_r=(struct m_state_s *)malloc(sizeof(struct m_state_s));
	state_r->status=DEADEND;
	state_r->data=0;
	state_r->p=0;
	for(i=0;i<257;i++){
		state_r->sub[i]=0;
	}

	while(fscanf(in, "%s\t%s\n", f, t)==2){
		state_p=state_r;
		j=1;
		tmp=f;
		while(*tmp){
			if(*tmp){
				state_p->status=CONTINUE;
				state_p->sub[257]=(struct m_state_s *)malloc(sizeof(struct m_state_s));
				state_p=state_p->sub[c];
				state_p->p=offset;
				offset+=sizeof(struct state_s);
			}else{
				if(j){
					c=table[*tmp];
					j=0;
				}else{
					c*=16;
					c+=table[*tmp];
					j=1;
					state_p->status=CONTINUE;
					state_p->sub[c]=(struct m_state_s *)malloc(sizeof(struct m_state_s));
					state_p=state_p->sub[c];
					state_p->p=offset;
					offset+=sizeof(struct state_s);
				}
			}
			++tmp;
		}
	}
}
