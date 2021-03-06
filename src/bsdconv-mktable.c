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

#ifdef DEBUG
#define DPRINTF(fmt, ...) do { fprintf(stderr, fmt , ## __VA_ARGS__); } while (0);
#else
#define DPRINTF(fmt, ...) do { } while (0);
#endif

#ifdef USE_FMALLOC
extern const char *fmalloc_template;
#endif

struct m_data_st{
	/* struct data_st */
	char *data;
	size_t len;
	offset_t next;

	/* extra */
	offset_t offset;
	struct m_data_st *n;
};

struct m_state_st{
	/* mapped to struct state_t*/
	char status;
	struct m_data_st *data;
	uint16_t beg;
	uint16_t end;
	struct m_state_st **base;

	/* extra */
	int level;
	offset_t offset;
	struct m_state_st *n;
};

struct m_state_st * state_new(int status){
	struct m_state_st *ret=(struct m_state_st *)FMALLOC(sizeof(struct m_state_st));
	ret->status=status;
	ret->n=NULL;
	ret->beg=~0;
	ret->end=0;
	ret->base=NULL;
	ret->data=NULL;
	return ret;
}

struct states{
	struct m_state_st *state;
	int upper_bound;
	int lower_bound;
	int level;
	struct states *n;
};

struct hash{
	uintptr_t v;
	int c;
	struct hash *parent;
	struct hash **sub;
	offset_t offset;
	void *head;
};

char table[256]={};
char ci_table[256]={0};

uintptr_t offset=0;
struct m_state_st *state_r, *state_t, holder;

struct hash *datalist_hash, *data_hash;

uintptr_t hash(int *p, uintptr_t l){
	int i;
	struct hash *hash_p=datalist_hash;
	struct hash *hash_q=data_hash;
	if(l==0)
		return 0;
	for(i=l-1;i>=0;--i){
		if(hash_p->sub==NULL)
			hash_p->sub=calloc(257, sizeof(struct hash *));
		if(hash_p->sub[p[i]]==NULL){
			hash_p->sub[p[i]]=FMALLOC(sizeof(struct hash));
			hash_p->sub[p[i]]->c=p[i];
			hash_p->sub[p[i]]->parent=hash_p;
			hash_p->sub[p[i]]->v=0;
			hash_p->sub[p[i]]->offset=0;
			hash_p->sub[p[i]]->sub=NULL;
		}
		//printf("hash_p: [%d]%X %p => %p\n", i, p[i], hash_p, hash_p->sub[p[i]]);
		if(p[i]==256){
			hash_p->v=(uintptr_t) hash_q;
			hash_p=hash_p->sub[p[i]];
			hash_q=data_hash;
		}else{
			hash_p=hash_p->sub[p[i]];
			if(hash_q->sub==NULL)
				hash_q->sub=calloc(257, sizeof(struct hash *));
			if(hash_q->sub[p[i]]==NULL){
				hash_q->sub[p[i]]=FMALLOC(sizeof(struct hash));
				hash_q->sub[p[i]]->c=p[i];
				hash_q->sub[p[i]]->parent=hash_q;
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
struct hash *callback_hash;

uintptr_t hash_callback(int *p, uintptr_t l){
	int i;
	struct hash *hash_q=callback_hash;
	for(i=l-1;i>=0;--i){
		if(hash_q->sub==NULL)
			hash_q->sub=calloc(257, sizeof(struct hash *));
		if(hash_q->sub[p[i]]==NULL){
			hash_q->sub[p[i]]=FMALLOC(sizeof(struct hash));
			hash_q->sub[p[i]]->c=p[i];
			hash_q->sub[p[i]]->parent=hash_q;
			hash_q->sub[p[i]]->v=0;
			hash_q->sub[p[i]]->offset=0;
			hash_q->sub[p[i]]->head=NULL;
			hash_q->sub[p[i]]->sub=NULL;
		}
		hash_q=hash_q->sub[p[i]];
	}
	if(hash_q->v==0){
		state_t->n=state_new(SUBROUTINE);
		state_t=state_t->n;
		hash_q->v=(uintptr_t) state_t;
	}
	return hash_q->v;
}

int main(int argc, char *argv[]){
	int i, j, k, c=0, cu, cl, case_insensitive, level;
	FILE *fp;
	char inbuf[1024], *f, *t, *tmp, *of, *ot;
	int dat[1024];
	offset_t sub[257];
	uintptr_t l;
	struct m_data_st *data_r=NULL, *data_p=NULL, *data_q=NULL, *data_t=NULL;
	struct states *todo=NULL, *newtodo, *newtodo_tail, *todo_item;
	struct state_st dstate;
	struct data_st ddata;
	struct hash *hash_p;
	int callback=0;
	void *tofree;
#ifdef USE_FMALLOC
	char *bsdconv_mktable_fmalloc_template;
#endif

	/* hex table */
	for(i=0;i<10;++i){
		table['0'+i]=i;
	}
	for(i=0;i<6;++i){
		table['a'+i]=10+i;
		table['A'+i]=10+i;
	}

	/* case insensitive table */
	for(i=0;i<=26;++i){
		ci_table['A'+i]='a'+i;
		ci_table['a']='A'+i;
	}

	if(argc!=3){
		fprintf(stderr, "Usage:\n\t%s inputfile outputfile\n", argv[0]);
		return 1;
	}

	printf("Making table %s\n", argv[1]);

#ifdef USE_FMALLOC
	bsdconv_mktable_fmalloc_template=malloc(strlen(argv[2])+10);
	sprintf(bsdconv_mktable_fmalloc_template, "%s.XXXXX", argv[2]);
	fmalloc_template=bsdconv_mktable_fmalloc_template;
#endif

	fp=fopen(argv[1], "r");
	if(!fp){
		fprintf(stderr, "Failed opening input file %s.\n", argv[1]);
		exit(1);
	}

	datalist_hash=FMALLOC(sizeof(struct hash));
	datalist_hash->sub=NULL;
	datalist_hash->parent=0;
	datalist_hash->v=0;
	datalist_hash->offset=0;
	datalist_hash->head=NULL;

	data_hash=FMALLOC(sizeof(struct hash));
	data_hash->sub=NULL;
	data_hash->parent=0;
	data_hash->v=0;
	data_hash->offset=0;
	data_hash->head=NULL;

	callback_hash=FMALLOC(sizeof(struct hash));
	callback_hash->sub=NULL;
	callback_hash->parent=0;
	callback_hash->v=0;
	callback_hash->offset=0;
	callback_hash->head=NULL;

	newtodo=malloc(sizeof(struct states));
	newtodo->n=NULL;
	newtodo->lower_bound=3;
	newtodo->upper_bound=2;
	newtodo_tail=newtodo;

	state_t=state_r=state_new(DEADEND);

	holder.beg=0;
	holder.end=1;
	holder.base=calloc(257, sizeof(struct m_state_st *));
	holder.base[0]=state_r;

	while(fgets((char *)inbuf, 1024, fp)){
		if(inbuf[0]=='#') continue;
		tmp=inbuf;
		f=of=strsep((char **)&tmp, "\t ");
		while(strchr("\t ",*tmp)){
			++tmp;
		}
		if(*tmp){
			t=ot=strsep((char **)&tmp, "\t\r\n# ");
		}else{
			t=ot=NULL;
		}

		todo=malloc(sizeof(struct states));
		todo->n=NULL;
		todo->state=&holder;
		todo->upper_bound=0;
		todo->lower_bound=0;
		todo->level=0;

		//read input sequence, build tree (not including leaves), generate todo list
		if(*f=='!'){
			case_insensitive=1;
			++f;
		}else{
			case_insensitive=0;
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
			todo_item=todo;
			while(todo){
				todo_item=todo;
				for(c=0;c<=256;++c){
					if(c>=todo_item->lower_bound && c<=todo_item->upper_bound){
						level=1;
					}else if(case_insensitive && ci_table[c] && ci_table[c]>=todo_item->lower_bound && ci_table[c]<=todo_item->upper_bound){
						level=0;
					}else{
						continue;
					}
					if(todo_item->state->base==NULL)
						todo_item->state->base=calloc(257, sizeof(struct m_state_st *));
					if(todo_item->state->base[c]){
						if(todo_item->state->base[c]->status==SUBROUTINE){
							state_t->n=state_new(SUBMATCH_SUBROUTINE);
							state_t=state_t->n;
							state_t->data=todo_item->state->base[c]->data;
							state_t->level=todo_item->state->base[c]->level;
DPRINTF("%p SUBROUTINE -> SUBMATCH_SUBROUTINE: %p\n", todo_item->state->base[c], state_t);
							todo_item->state->base[c]=state_t;
						}else if(todo_item->state->base[c]->status==MATCH){
							todo_item->state->base[c]->status=SUBMATCH;
DPRINTF("%p MATCH -> SUBMATCH\n", todo_item->state);
						}
					}else{

						state_t->n=todo_item->state->base[c]=state_new(CONTINUE);
						state_t=state_t->n;
DPRINTF("%p[%X]=%p\n", todo_item->state, c, state_t);
					}
					if(c < todo_item->state->beg)
						todo_item->state->beg=c;
					if(c >= todo_item->state->end)
						todo_item->state->end=c+1;
					newtodo_tail->n=malloc(sizeof(struct states));
					newtodo_tail=newtodo_tail->n;
					newtodo_tail->state=todo_item->state->base[c];
					newtodo_tail->lower_bound=cl;
					newtodo_tail->upper_bound=cu;
					newtodo_tail->level=todo_item->level+level;
					newtodo_tail->n=NULL;
				}
				todo=todo->n;
				free(todo_item);
			}
			todo=newtodo->n;
			newtodo->n=NULL;
			newtodo_tail=newtodo;
			++f;
		}

		//parse output data
		j=0;
		l=0;
		callback=0;
		if(*t=='?'){
			t+=1;
			callback=1;
		}
		while(*t){
			if(*t==','){
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

		//process todo list, associate leaves with output sequence's hash id
		while(todo){
			todo_item=todo;
			for(c=0;c<=256;++c){
				if(c>=todo_item->lower_bound && c<=todo_item->upper_bound){
					level=1;
				}else if(case_insensitive && ci_table[c] && ci_table[c]>=todo_item->lower_bound && ci_table[c]<=todo_item->upper_bound){
					level=0;
				}else{
					continue;
				}
				if(c < todo_item->state->beg)
					todo_item->state->beg=c;
				if(c >= todo_item->state->end)
					todo_item->state->end=c+1;
				if(todo_item->state->base==NULL)
					todo_item->state->base=calloc(257, sizeof(struct m_state_st *));
				if(todo_item->state->base[c]){
					if(
						(
							todo_item->state->base[c]->status==MATCH
							||
							todo_item->state->base[c]->status==SUBMATCH
							||
							(
								(
								todo_item->state->base[c]->status==SUBROUTINE
								||
								todo_item->state->base[c]->status==SUBMATCH_SUBROUTINE
								)
								&&
								callback
							)
						)
						&&
						(level+todo_item->level) <= todo_item->state->base[c]->level
					){
						DPRINTF("Already has data at %p for %s, dropping %s\n", todo_item->state->base[c], of, ot);
						continue;
					}else if(todo_item->state->base[c]->status==CONTINUE){
						if(callback){
							todo_item->state->base[c]->status=SUBMATCH_SUBROUTINE;
							DPRINTF("%p CONTINUE -> SUBMATCH_SUBROUTINE\n", todo_item->state->base[c]);
						}else{
							todo_item->state->base[c]->status=SUBMATCH;
							DPRINTF("%p CONTINUE -> SUBMATCH\n", todo_item->state->base[c]);
						}
					}else if(!callback){
						if(todo_item->state->base[c]->status==SUBROUTINE){
							state_t->n=state_new(MATCH);
							state_t=state_t->n;
							state_t->data=todo_item->state->base[c]->data;
							state_t->level=todo_item->state->base[c]->level;
							DPRINTF("%p SUBROUTINE -> MATCH: %p\n", todo_item->state->base[c], state_t);
							todo_item->state->base[c]=state_t;
						}else{
							DPRINTF("%p %d -> MATCH for %s\n", todo_item->state->base[c], todo_item->state->base[c]->status, ot);
							todo_item->state->base[c]->status=MATCH;
						}
					}
				}else if(callback){
					todo_item->state->base[c]=(struct m_state_st *) hash_callback(dat, l);
				}else{
					todo_item->state->base[c]=state_t->n=state_new(MATCH);
					state_t=state_t->n;
				}
				todo_item->state->base[c]->data=(struct m_data_st *)hash(dat,l);
				todo_item->state->base[c]->level=level+todo_item->level;
			}
			todo=todo->n;
			free(todo_item);
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
	fp=fopen(argv[2], "wb+");
	if(!fp){
		fprintf(stderr, "Failed opening input file %s.\n", argv[1]);
		exit(1);
	}
	state_t=state_r;
	data_p=NULL;
	while(state_t){
		hash_p=(struct hash *)state_t->data;
		k=1;	//begin of data cell
		l=0;	//length counter
		j=0;	//has read something to proceed
		data_q=NULL;	//pointer of last data cell
		while(hash_p && hash_p->parent){
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
			hash_p=hash_p->parent;
		}
		if(j){
			j=0;
			data_p->len=l;
		}
		hash_p=(struct hash *)state_t->data;
		if(hash_p)
			dstate.data=en_offset(hash_p->offset);
		else
			dstate.data=0;
		dstate.status=state_t->status;
		dstate.beg=en_uint16(state_t->beg);
		dstate.end=en_uint16(state_t->end);
		if(state_t->base){
			dstate.base=en_offset(offset);
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
		state_t=state_t->n;
	}

	state_t=state_r;
	while(state_t){
		tofree=state_t;
		state_t=state_t->n;
		FFREE(tofree);
	}

	data_t=data_r;
	while(data_t){
		hash_p=(struct hash *)data_t->data;
		if(hash_p && !hash_p->head){
			hash_p->head=data_t->data;
			hash_p=hash_p->parent;
			while(hash_p && hash_p->parent){
				hash_p->head=data_t->data;
				hash_p=hash_p->parent;
			}
		}
		data_t=data_t->n;
	}

	data_t=data_r;
	while(data_t){
		hash_p=(struct hash *)data_t->data;
		if(hash_p && !hash_p->offset){
			fseek(fp, offset, SEEK_SET);
			hash_p=hash_p->head;
			while(hash_p && hash_p->parent){
				hash_p->offset=offset;
				//printf("Writing byte: %X.\n", hash_p->c);
				fputc(hash_p->c, fp);
				offset+=1;
				hash_p=hash_p->parent;
			}
		}
		hash_p=(struct hash *)data_t->data;
		if(hash_p)
			ddata.data=en_offset(hash_p->offset);
		else
			ddata.data=en_offset(0);
		ddata.len=data_t->len;
		ddata.next=en_offset(data_t->next);
		fseek(fp, data_t->offset, SEEK_SET);
		//printf("Writing struct data_st at %p %c %p.\n", (void *)(uintptr_t)data_t->offset, hash_p->parent->c, ddata.next);
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
