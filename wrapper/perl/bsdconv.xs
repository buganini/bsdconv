#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <bsdconv.h>
#include <errno.h>
#include <string.h>

#define IBUFLEN 1024
#define OBUFLEN 1024

MODULE = bsdconv		PACKAGE = bsdconv

SV*
conv_once(c,str)
	char *c
	SV* str
	PREINIT:
		struct bsdconv_instance *ins;
		char *s;
		SSize_t l;
	CODE:
		ins=bsdconv_create(c);
		if(ins==NULL) XSRETURN_UNDEF;
		s=SvPV(str, l);
		ins->mode=BSDCONV_CC;
		ins->feed=s;
		ins->feed_len=l;
		bsdconv_init(ins);
		bsdconv(ins);
		RETVAL=newSVpv(ins->back, (STRLEN)ins->back_len);
		free(ins->back);
		bsdconv_destroy(ins);
	OUTPUT:
		RETVAL

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
		ins->mode=BSDCONV_CC;
		ins->feed=s;
		ins->feed_len=l;
		bsdconv_init(ins);
		bsdconv(ins);
		RETVAL=newSVpv(ins->back, (STRLEN)ins->back_len);
		free(ins->back);
	OUTPUT:
		RETVAL

SV*
conv_file(i, f1, f2)
	IV i
	SV* f1
	SV* f2
	PREINIT:
		struct bsdconv_instance *p;
		char *s1, *s2;
		SSize_t l;
		int r;
		FILE *inf, *otf;
		unsigned char in[IBUFLEN], out[OBUFLEN];
	CODE:
		p=INT2PTR(struct bsdconv_instance *, i);
		s1=SvPV(f1, l);
		s2=SvPV(f2, l);
		inf=fopen(s1,"r");
		if(!inf) XSRETURN_UNDEF;
		otf=fopen(s2,"w");
		if(!otf) XSRETURN_UNDEF;
		p->in_buf=in;
		p->in_len=IBUFLEN;
		p->out_buf=out;
		p->out_len=OBUFLEN;
		p->mode=BSDCONV_BB;
		bsdconv_init(p);
		do{
			if(p->feed_len) p->feed_len=fread(p->feed, 1, p->feed_len, inf);
			r=bsdconv(p);
			if(p->back_len)fwrite(p->back, 1, p->back_len, otf);
		}while(r);
		fclose(inf);
		fclose(otf);
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
		struct bsdconv_instance *ins;
		char *s;
		SSize_t l;
	CODE:
		s=bsdconv_error();
		RETVAL=newSVpv(s, (STRLEN)strlen(s));
		free(s);
	OUTPUT:
		RETVAL
