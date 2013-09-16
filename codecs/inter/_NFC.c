/*
 * Some code and table come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 * Copyright (c) 2013 Kuan-Chung Chiu <buganini@gmail.com>
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

#include "../../src/bsdconv.h"

#include "_NF-CCC.h"

struct my_s {
	struct bsdconv_instance *map;
	size_t *err;
	int status;
	struct data_rt *starter;
	int ccc;
	struct data_rt *pending_head;
	struct data_rt *pending_tail;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	r->map=bsdconv_create("_NFC-MAP,COUNT#ERR");
	r->err=bsdconv_counter(r->map, "ERR");
	r->starter=NULL;
	r->pending_tail=r->pending_head=malloc(sizeof(struct data_rt));
	r->pending_head->flags=0;
	r->pending_tail->next=NULL;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->status=0;
	struct data_rt *t;
	if(r->starter){
		DATA_FREE(r->starter);
	}
	while(r->pending_head->next){
		t=r->pending_head->next->next;
		DATA_FREE(r->pending_head->next);
		r->pending_head->next=t;
	}
	r->pending_tail=r->pending_head;
}

void cbconv(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	unsigned char *data;
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct bsdconv_phase *prev_phase=&ins->phase[ins->phase_index-1];
	data=this_phase->curr->data;
	int i;
	uint32_t ucs=0;
	int ccc=0;
	int max=sizeof(ccc_table) / sizeof(struct ccc_interval) - 1;
	int min = 0;
	int mid;
	struct data_rt *t;

	if(data[0]==0x1){
		for(i=1;i<this_phase->curr->len;++i){
			ucs<<=8;
			ucs|=data[i];
		}
		if (ucs < ccc_table[0].beg || ucs > ccc_table[max].end){
			//noop
		}else while (max >= min) {
			mid = (min + max) / 2;
			if (ucs > ccc_table[mid].end)
				min = mid + 1;
			else if (ucs < ccc_table[mid].beg)
				max = mid - 1;
			else{
				ccc=ccc_table[mid].ccc;
				break;
			}
		}
		switch(r->status){
			case 0:
				r->ccc=0;
				if(ccc==0){
					if(r->starter){
						DATA_FREE(r->starter);
					}
					r->status=1;
					DATA_MALLOC(r->starter);
					*(r->starter)=*(this_phase->curr);
					this_phase->curr->flags &= ~F_FREE;
					this_phase->state.status=SUBMATCH;
				}else{
					this_phase->state.status=DEADEND;
				}
				return;
			case 1:
				if(ccc==0){
					r->ccc=0;
					if(r->pending_head->next==NULL){ //adjacent starters
						bsdconv_init(r->map);
						bsdconv_counter_reset(r->map, NULL);
						r->map->input=*(r->starter);
						r->map->input.flags &= ~F_FREE;
						DATA_MALLOC(r->map->input.next);
						*(r->map->input.next)=*(this_phase->curr);
						r->map->input.next->flags &= ~F_FREE;
						r->map->input.next->next=NULL;
						r->map->flush=1;
						bsdconv(r->map);
						if(*(r->err)==0){
							DATA_FREE(r->starter);
							r->status=1;
							r->starter=r->map->phase[r->map->phasen].data_head->next;
							r->map->phase[r->map->phasen].data_head->next=NULL;
						}else{
							cbflush(ins);
							r->status=1;
							DATA_MALLOC(r->starter);
							*(r->starter)=*(this_phase->curr);
							this_phase->curr->flags &= ~F_FREE;
						}
					}else{
						cbflush(ins);
						r->status=1;
						DATA_MALLOC(r->starter);
						*(r->starter)=*(this_phase->curr);
						this_phase->curr->flags &= ~F_FREE;
					}
				}else{
					if(ccc <= r->ccc){ //blocked
						r->ccc=ccc;
						DATA_MALLOC(r->pending_tail->next);
						r->pending_tail=r->pending_tail->next;
						*(r->pending_tail)=*(this_phase->curr);
						r->pending_tail->next=NULL;
						this_phase->curr->flags &= ~F_FREE;
					}else{
						bsdconv_init(r->map);
						bsdconv_counter_reset(r->map, NULL);
						r->map->input=*(r->starter);
						r->map->input.flags &= ~F_FREE;
						DATA_MALLOC(r->map->input.next);
						*(r->map->input.next)=*(this_phase->curr);
						r->map->input.next->flags &= ~F_FREE;
						r->map->input.next->next=NULL;
						r->map->flush=1;

						bsdconv(r->map);
						if(*(r->err)==0){
							DATA_FREE(r->starter);
							r->status=1;
							r->starter=r->map->phase[r->map->phasen].data_head->next;
							r->map->phase[r->map->phasen].data_head->next=NULL;

							if(r->pending_head->next){
								t=this_phase->curr->next;
								this_phase->curr->next=r->pending_head->next;
								r->pending_tail->next=t;
								r->pending_tail=r->pending_head;
								r->pending_head->next=NULL;
								while(prev_phase->data_tail->next){
									prev_phase->data_tail=prev_phase->data_tail->next;
								}
							}
							r->ccc=0;
						}else{
							r->ccc=ccc;
							DATA_MALLOC(r->pending_tail->next);
							r->pending_tail=r->pending_tail->next;
							*(r->pending_tail)=*(this_phase->curr);
							r->pending_tail->next=NULL;
							this_phase->curr->flags &= ~F_FREE;
						}
					}
				}
				this_phase->state.status=SUBMATCH;
				return;
		}
	}else{
		r->ccc=0;
		cbflush(ins);
		DATA_MALLOC(this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		*(this_phase->data_tail)=*(this_phase->curr);
		this_phase->curr->flags &= ~F_FREE;
		this_phase->data_tail->next=NULL;
	}
	return;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct data_rt *t;
	bsdconv_destroy(r->map);
	if(r->status){
		DATA_FREE(r->starter);
	}
	while(r->pending_head){
		t=r->pending_head->next;
		DATA_FREE(r->pending_head);
		r->pending_head=t;
	}
	free(r);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct data_rt *t;

	this_phase->state.status=NEXTPHASE;

	if(r->status==0)
		return;

	this_phase->data_tail->next=r->starter;
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;

	r->starter=NULL;

	while(r->pending_head->next){
		t=r->pending_head->next->next;
		this_phase->data_tail->next=r->pending_head->next;
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		r->pending_head->next=t;
	}
	r->pending_tail=r->pending_head;

	r->status=0;
}
