#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <bsdconv.h>

MODULE = bsdconv		PACKAGE = bsdconv

IV
create(conversion)
	char* conversion
	PREINIT:
		struct bsdconv_instance *ins;
	CODE:
		ins=bsdconv_create(conversion);
		printf("%p\n",ins);
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
		printf("%p\n",ins);
		s=SvPV(str, l);
		ins->mode=BSDCONV_CC;
		ins->feed=s;
		ins->feed_len=l;
		bsdconv_init(ins);
		bsdconv(ins);
		printf("%u", (unsigned int)ins->back_len);
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
