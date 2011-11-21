//This file is identical to FROM_ALIAS.c
/*
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"


void cbcreate(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	ins->phase[ins->phase_index].codec[this_phase->index].priv=bsdconv_create("ASCII:PASS");
}

void cbdestroy(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	void *p=ins->phase[ins->phase_index].codec[this_phase->index].priv;
	bsdconv_destroy(p);
}

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct bsdconv_instance *uni=this_phase->codec[this_phase->index].priv;
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
