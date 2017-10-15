#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"


int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	THIS_CODEC(ins)->priv=bsdconv_create("ASCII:PASS");
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	bsdconv_destroy(THIS_CODEC(ins)->priv);
}

#define CNS11643_UNICODE "CNS11643-UNICODE"

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase = THIS_PHASE(ins);
	struct bsdconv_instance *uni = THIS_CODEC(ins)->priv;
  int found = 0;
	const char *s;

  if(bsdconv_module_check(INTER, CNS11643_UNICODE)){
    found += 1;
    s = CNS11643_UNICODE;
    bsdconv_init(uni);
  	uni->input.data=strdup(s);
  	uni->input.len=strlen(s);
  	uni->input.flags=F_FREE;
  	uni->input.next=NULL;
  	uni->flush=1;
  	bsdconv(uni);
  	this_phase->data_tail->next=uni->phase[uni->phasen].data_head->next;
  	uni->phase[uni->phasen].data_head->next=NULL;
  	uni->phase[uni->phasen].data_tail=uni->phase[uni->phasen].data_head;
  	while(this_phase->data_tail->next!=NULL){
  		this_phase->data_tail=this_phase->data_tail->next;
  	}
  }

  if(found==0){
    s = "PASS";
    bsdconv_init(uni);
  	uni->input.data=strdup(s);
  	uni->input.len=strlen(s);
  	uni->input.flags=F_FREE;
  	uni->input.next=NULL;
  	uni->flush=1;
  	bsdconv(uni);
  	this_phase->data_tail->next=uni->phase[uni->phasen].data_head->next;
  	uni->phase[uni->phasen].data_head->next=NULL;
  	uni->phase[uni->phasen].data_tail=uni->phase[uni->phasen].data_head;
  	while(this_phase->data_tail->next!=NULL){
  		this_phase->data_tail=this_phase->data_tail->next;
  	}
  }

	this_phase->state.status=NEXTPHASE;
	return;
}
