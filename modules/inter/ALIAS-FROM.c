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

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct bsdconv_instance *uni=THIS_CODEC(ins)->priv;
	const char *locale;
	const char *s;

	if (((locale=getenv("LC_ALL")) || (locale=getenv("LC_CTYPE")) || (locale=getenv ("LANG"))) && ((s=strstr(locale, "."))!=NULL)){
		s+=1;
	}else{
		s=locale;
	}
	if(s==NULL || *s==0 || strcmp(s, "C")==0 || strcmp(s, "POSIX")==0){
		s="ASCII";
	}
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

	this_phase->state.status=NEXTPHASE;
	return;
}
