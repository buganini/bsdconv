/*
 * Copyright (c) 2009,2010 Kuan-Chung Chiu <buganini@gmail.com>
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

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <bsdconv.h>
#include <errno.h>
#include <string.h>

#define IBUFLEN 1024

MODULE = bsdconv		PACKAGE = bsdconv

IV
create(conversion)
	char* conversion
	PREINIT:
		struct bsdconv_instance *ins;
	CODE:
		ins=bsdconv_create(conversion);
		if(ins==NULL) XSRETURN_UNDEF;
		RETVAL=PTR2IV(ins);
	OUTPUT:
		RETVAL

SV*
conv(p, str)
	IV p
	SV* str
	PREINIT:
		struct bsdconv_instance *ins;
		char *s;
		SSize_t l;
	CODE:
		ins=INT2PTR(struct bsdconv_instance *, p);
		s=SvPV(str, l);

		bsdconv_init(ins);
		ins->output_mode=BSDCONV_AUTOMALLOC;
		ins->input.data=s;
		ins->input.len=l;
		ins->input.flags=0;
		ins->flush=1;
		bsdconv(ins);

		RETVAL=newSVpvn(ins->output.data, (STRLEN)ins->output.len);
		free(ins->output.data);
	OUTPUT:
		RETVAL

SV*
conv_file(i, f1, f2)
	IV i
	SV* f1
	SV* f2
	PREINIT:
		struct bsdconv_instance *ins;
		char *s1, *s2;
		SSize_t l;
		FILE *inf, *otf;
		char *in;
		char *tmp;
	CODE:
		ins=INT2PTR(struct bsdconv_instance *, i);
		s1=SvPV(f1, l);
		s2=SvPV(f2, l);
		inf=fopen(s1,"r");
		if(!inf) XSRETURN_UNDEF;
		tmp=malloc(l+8);
		strcpy(tmp, s2);
		strcat(tmp, ".XXXXXX");
		if(mktemp(tmp)==NULL){
			free(tmp);
			XSRETURN_UNDEF;
		}
		otf=fopen(tmp,"w");
		if(!otf){
			free(tmp);
			XSRETURN_UNDEF;
		}

		bsdconv_init(ins);
		do{
			in=malloc(IBUFLEN);
			ins->input.data=in;
			ins->input.len=fread(in, 1, IBUFLEN, inf);
			ins->input.flags|=F_FREE;
			if(ins->input.len==0){
				ins->flush=1;
			}
			ins->output_mode=BSDCONV_FILE;
			ins->output.data=otf;
			bsdconv(ins);
		}while(ins->flush==0);

		fclose(inf);
		fclose(otf);
		unlink(s2);
		rename(tmp,s2);
		free(tmp);
		XSRETURN_YES;
	OUTPUT:
		RETVAL

void
destroy(p)
	IV p
	PREINIT:
		struct bsdconv_instance *ins;
	CODE:
		ins=INT2PTR(struct bsdconv_instance *,p);
		bsdconv_destroy(ins);

HV*
info(p)
	IV p
	PREINIT:
		struct bsdconv_instance *ins;
	CODE:
		ins=INT2PTR(struct bsdconv_instance *, p);
		RETVAL=newHV();
		sv_2mortal((SV*)RETVAL);
		hv_store(RETVAL, "ierr", 4, newSVuv(ins->ierr), 0);
		hv_store(RETVAL, "oerr", 4, newSVuv(ins->oerr), 0);
	OUTPUT:
		RETVAL

SV*
error()
	PREINIT:
		char *s;
	CODE:
		s=bsdconv_error();
		RETVAL=newSVpv(s, 0);
		free(s);
	OUTPUT:
		RETVAL
