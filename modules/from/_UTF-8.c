#include <stdlib.h>
#include "../../src/bsdconv.h"

// Ref: https://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

struct my_s{
	int status;
	ucs_t lead_surrogate;
	ucs_t ucs;
	int cesu;
	int loose;
	int nul;
	int overlong;
	int super;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r = malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv = r;
	r->cesu = 0;
	r->loose = 0;
	r->nul = 0;
	r->overlong = 0;
	r->super = 0;

	while(arg){
		if(strcasecmp(arg->key, "CESU")==0){
			r->cesu = 1;
		}else if(strcasecmp(arg->key, "LOOSE")==0){
			r->loose = 1;
		}else if(strcasecmp(arg->key, "NUL")==0){
			r->nul = 1;
		}else if(strcasecmp(arg->key, "OVERLONG")==0){
			r->overlong = 1;
		}else if(strcasecmp(arg->key, "SUPER")==0){
			r->super = 1;
		}
		arg=arg->next;
	}
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	r->status = 0;
	r->lead_surrogate.ucs4 = 0;
	r->ucs.ucs4 = 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	free(r);
}

#define DEADEND() do{	\
	t->status = 0;	\
	t->lead_surrogate.ucs4 = 0;	\
	t->ucs.ucs4 = 0;	\
	this_phase->state.status=DEADEND;	\
	return;	\
}while(0)

#define COMMIT() do{	\
	this_phase->state.status=NEXTPHASE;	\
	t->status = 0;	\
	uint32_t ucs = be32toh(t->ucs.ucs4);	\
	if(ucs >= 0xD800 && ucs <= 0xDBFF){	\
		if(t->cesu){	\
			if(t->lead_surrogate.ucs4){	\
				if(t->loose){	\
					PASS(t->lead_surrogate);	\
				}else{	\
					DEADEND();	\
				}	\
			}	\
			t->lead_surrogate.ucs4 = t->ucs.ucs4;	\
			t->ucs.ucs4 = 0;	\
			if(t->loose){	\
				this_phase->state.status=SUBMATCH;	\
			}else{	\
				this_phase->state.status=CONTINUE;	\
			}	\
		}else if(t->loose){	\
			PASS(t->ucs);	\
			t->ucs.ucs4 = 0;	\
		}else{	\
			DEADEND();	\
		}	\
	}else if(ucs >= 0xDC00 && ucs <= 0xDFFF){	\
		if(t->cesu){	\
			if(t->lead_surrogate.ucs4){	\
				uint32_t cp = 0x10000;	\
				cp |= ((be32toh(t->lead_surrogate.ucs4) - 0xD800) << 10) & bb11111111110000000000;	\
				cp |= (ucs - 0xDC00) & bb1111111111;	\
				t->ucs.ucs4 = htobe32(cp);	\
				PASS(t->ucs);	\
				t->lead_surrogate.ucs4 = 0;	\
				t->ucs.ucs4 = 0;	\
			}else if(t->loose){	\
				PASS(t->ucs);	\
				t->ucs.ucs4 = 0;	\
			}else{	\
				DEADEND();	\
			}	\
		}else if(t->loose){	\
			PASS(t->ucs);	\
			t->ucs.ucs4 = 0;	\
		}else{	\
			DEADEND();	\
		}	\
	}else{	\
		if(t->lead_surrogate.ucs4){	\
			if(t->loose){	\
				PASS(t->lead_surrogate);	\
				t->lead_surrogate.ucs4 = 0;	\
			}else{	\
				DEADEND();	\
			}	\
		}	\
		PASS(t->ucs);	\
		t->ucs.ucs4 = 0;	\
	}	\
	return;	\
}while(0)

#define PASS(x) do{	\
	if((x).ucs4==0 && !t->nul){	\
		DEADEND();	\
	}	\
	int i;	\
	for(i=0;i<3 /* instead of 4, to map NUL to 0100 */;i+=1){	\
		if((x).byte[i] != 0){	\
			break;	\
		}	\
	}	\
	int len = 4 - i + 1;	\
	char *buf = malloc(len);	\
	buf[0] = 0x01;	\
	int p = 1;	\
	while(i<4){	\
		buf[p] = (x).byte[i];	\
		i += 1;	\
		p += 1;	\
	}	\
	DATA_MALLOC(ins, this_phase->data_tail->next);	\
	this_phase->data_tail=this_phase->data_tail->next;	\
	this_phase->data_tail->next=NULL;	\
	this_phase->data_tail->len=len;	\
	this_phase->data_tail->flags=F_FREE;	\
	this_phase->data_tail->data=buf;	\
}while(0)

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	if(t->lead_surrogate.ucs4 && t->loose){
		PASS(t->lead_surrogate);
		t->lead_surrogate.ucs4 = 0;
	}
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *t=THIS_CODEC(ins)->priv;
	unsigned char d;

	for(;this_phase->i<this_phase->curr->len;this_phase->i+=1){
		d=UCP(this_phase->curr->data)[this_phase->i];
		switch(t->status){
			case 0:
				if((d & bb10000000) == 0){ // *0₃₃₃₃₃₃₃, total 7
					/* exclude ASCII */
					DEADEND();

					/* Unreachable */
					t->ucs.ucs4 = d;
					COMMIT();
				}else if((d & bb11100000) == bb11000000){ // *110₂₂₂₃₃ 10₃₃₃₃₃₃, total 11 (+4)
					t->status = 21;
					t->ucs.byte[2] |= (d >> 2) & bb00000111;
					t->ucs.byte[3] |= (d << 6) & bb10000000;
					if(!t->overlong && t->ucs.ucs4==0){
						DEADEND();
					}
					t->ucs.byte[3] |= (d << 6) & bb01000000;
				}else if((d & bb11110000) == bb11100000){ // *1110₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 16 (+5)
					t->status = 31;
					t->ucs.byte[2] |= (d << 4) & bb11110000;
				}else if((d & bb11111000) == bb11110000){ // *11110₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 21 (+5)
					t->status = 41;
					t->ucs.byte[1] |= (d << 2) & bb00011100;
				}else if((d & bb11111100) == bb11111000){ // *111110₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 26 (+5)
					if(!t->super){
						DEADEND();
					}
					t->status = 51;
					t->ucs.byte[0] |= d & bb00000011;
				}else if((d & bb11111110) == bb11111100){ // *1111110₀ 10₀₀₀₀₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 31 (+5)
					if(!t->super){
						DEADEND();
					}
					t->status = 61;
					t->ucs.byte[0] |= (d << 6) & bb01000000;
				}else{
					DEADEND();
				}
				break;
			case 21:
				if((d & bb11000000) == bb10000000){ // 110₂₂₂₃₃ *10₃₃₃₃₃₃, total 11 (+4)
					t->ucs.byte[3] |= d & bb00111111;
					COMMIT();
				}else{
					DEADEND();
				}
				break;
			case 31:
				if((d & bb11000000) == bb10000000){ // 1110₂₂₂₂ *10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 16 (+5)
					t->status = 32;
					t->ucs.byte[2] |= (d >> 2) & bb00001000;
					if(!t->overlong && t->ucs.ucs4==0){
						DEADEND();
					}
					t->ucs.byte[2] |= (d >> 2) & bb00000111;
					t->ucs.byte[3] |= (d << 6) & bb11000000;
				}else{
					DEADEND();
				}
				break;
			case 32:
				if((d & bb11000000) == bb10000000){ // 1110₂₂₂₂ 10₂₂₂₂₃₃ *10₃₃₃₃₃₃, total 16 (+5)
					t->ucs.byte[3] |= d & bb00111111;
					COMMIT();
				}else{
					DEADEND();
				}
				break;
			case 41:
				if((d & bb11000000) == bb10000000){ // 11110₁₁₁ *10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 21 (+5)
					t->status = 42;
					t->ucs.byte[1] |= (d >> 4) & bb00000011;
					if(!t->overlong && t->ucs.ucs4==0){
						DEADEND();
					}
					t->ucs.byte[2] |= (d << 4) & bb11110000;
				}else{
					DEADEND();
				}
				break;
			case 42:
				if((d & bb11000000) == bb10000000){ // 11110₁₁₁ 10₁₁₂₂₂₂ *10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 21 (+5)
					t->status = 43;
					t->ucs.byte[2] |= (d >> 2) & bb00001111;
					t->ucs.byte[3] |= (d << 6) & bb11000000;
				}else{
					DEADEND();
				}
				break;
			case 43:
				if((d & bb11000000) == bb10000000){ // 11110₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ *10₃₃₃₃₃₃, total 21 (+5)
					t->ucs.byte[3] |= d & bb00111111;
					COMMIT();
				}else{
					DEADEND();
				}
				break;
			case 51:
				if((d & bb11000000) == bb10000000){ // 111110₀₀ *10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 26 (+5)
					t->status = 52;
					t->ucs.byte[1] |= (d << 2) & bb11100000;
					if(!t->overlong && t->ucs.ucs4==0){
						DEADEND();
					}
					t->ucs.byte[1] |= (d << 2) & bb00011100;
				}else{
					DEADEND();
				}
				break;
			case 52:
				if((d & bb11000000) == bb10000000){ // 111110₀₀ 10₁₁₁₁₁₁ *10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 26 (+5)
					t->status = 53;
					t->ucs.byte[1] |= (d >> 4) & bb00000011;
					t->ucs.byte[2] |= (d << 4) & bb11110000;
				}else{
					DEADEND();
				}
				break;
			case 53:
				if((d & bb11000000) == bb10000000){ // 111110₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ *10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 26 (+5)
					t->status = 54;
					t->ucs.byte[2] |= (d >> 2) & bb00001111;
					t->ucs.byte[3] |= (d << 6) & bb11000000;
				}else{
					DEADEND();
				}
				break;
			case 54:
				if((d & bb11000000) == bb10000000){ // 111110₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ *10₃₃₃₃₃₃, total 26 (+5)
					t->ucs.byte[3] |= d & bb00111111;
					COMMIT();
				}else{
					DEADEND();
				}
				break;
			case 61:
				if((d & bb11000000) == bb10000000){ // 1111110₀ *10₀₀₀₀₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 31 (+5)
					t->status = 62;
					t->ucs.byte[0] |= d & bb00111100;
					if(!t->overlong && t->ucs.ucs4==0){
						DEADEND();
					}
					t->ucs.byte[0] |= d & bb00000011;
				}else{
					DEADEND();
				}
				break;
			case 62:
				if((d & bb11000000) == bb10000000){ // 1111110₀ 10₀₀₀₀₀₀ *10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 31 (+5)
					t->status = 63;
					t->ucs.byte[1] |= (d << 2) & bb11111100;
				}else{
					DEADEND();
				}
				break;
			case 63:
				if((d & bb11000000) == bb10000000){ // 1111110₀ 10₀₀₀₀₀₀ 10₁₁₁₁₁₁ *10₁₁₂₂₂₂ 10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 31 (+5)
					t->status = 64;
					t->ucs.byte[1] |= (d >> 4) & bb00000011;
					t->ucs.byte[2] |= (d << 4) & bb11110000;
				}else{
					DEADEND();
				}
				break;
			case 64:
				if((d & bb11000000) == bb10000000){ // 1111110₀ 10₀₀₀₀₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ *10₂₂₂₂₃₃ 10₃₃₃₃₃₃, total 31 (+5)
					t->status = 65;
					t->ucs.byte[2] |= (d >> 2) & bb00001111;
					t->ucs.byte[3] |= (d << 6) & bb11000000;
				}else{
					DEADEND();
				}
				break;
			case 65:
				if((d & bb11000000) == bb10000000){ // 1111110₀ 10₀₀₀₀₀₀ 10₁₁₁₁₁₁ 10₁₁₂₂₂₂ 10₂₂₂₂₃₃ *10₃₃₃₃₃₃, total 31 (+5)
					t->ucs.byte[3] |= d & bb00111111;
					COMMIT();
				}else{
					DEADEND();
				}
				break;
			default:
				DEADEND();
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
