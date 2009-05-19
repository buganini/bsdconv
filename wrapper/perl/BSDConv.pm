package BSDConv;
use Inline C => Config => LIBS => '-L/usr/local/lib -lbsdconv';
use Inline C => Config => INC  => '-I/usr/local/include';
use Inline C;
use strict;

sub conv{
	my $cd=shift;
	my $s=shift;
	return _conv($cd, $s, length($s));
}

1;
__DATA__
__C__
#include <bsdconv.h>

SV* create(char *c){
	SV* r=bsdconv_create(c);
	return r;
}

char* _conv(SV* cd, SV* s, int len) {
	struct bsdconv_instruction ins;
	SV* ret;

	int r;
	ins.in_buf=SvPVX(s);
	ins.in_len=len;
	ins.mode=BSDCONV_BB;
	bsdconv_init( ((struct bsdconv_t *)SvIV(SvRV(cd))), &ins);
	do{
		r=bsd_conv( ((struct bsdconv_t *)SvIV(SvRV(cd))), &ins);
	}while(r);

	ret=newSVpvn(ins.back, ins.back_len);
	return ret;
}

void destroy(SV* cd){
	return bsdconv_destroy(((struct bsdconv_t *)SvIV(SvRV(cd))));
}
