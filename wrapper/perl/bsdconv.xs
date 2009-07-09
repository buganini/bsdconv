#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <bsdconv.h>
#include <errno.h>
#include <string.h>

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
		ins->mode=BSDCONV_CC;
		ins->feed=s;
		ins->feed_len=l;
		bsdconv_init(ins);
		bsdconv(ins);
		RETVAL=newSVpv(ins->back, (STRLEN)ins->back_len);
		free(ins->back);
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
