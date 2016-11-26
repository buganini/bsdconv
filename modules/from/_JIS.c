#include <stdlib.h>
#include <string.h>
#include "../../src/bsdconv.h"

// Ref: http://home.e05.itscom.net/t-mattsu/font/font3.html

enum {
	MODE_ASCII,
	MODE_JIS0201,
	MODE_JIS0208,
	MODE_JIS0212,
};

struct my_s{
	int mode;
	int state;
	struct bsdconv_codec jis0201;
	struct bsdconv_codec jis0208;
	struct bsdconv_codec jis0212;
	struct state_rt cstate;
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r = malloc(sizeof(struct my_s));
	THIS_CODEC(ins)->priv = r;
	r->mode = MODE_ASCII;
	r->state = 0;
	r->jis0201.desc="_JIS0201";
	r->jis0208.desc="_JIS0208";
	r->jis0212.desc="_JIS0212";
	if(!loadcodec(&r->jis0201, FROM)){
		return 1;
	}
	if(!loadcodec(&r->jis0208, FROM)){
		unloadcodec(&r->jis0201);
		return 1;
	}
	if(!loadcodec(&r->jis0212, FROM)){
		unloadcodec(&r->jis0201);
		unloadcodec(&r->jis0208);
		return 1;
	}

	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r = THIS_CODEC(ins)->priv;
	unloadcodec(&r->jis0201);
	unloadcodec(&r->jis0208);
	unloadcodec(&r->jis0212);
	free(r);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase = THIS_PHASE(ins);
	struct my_s *r = THIS_CODEC(ins)->priv;

	unsigned char d = UCP(this_phase->curr->data)[this_phase->i];
	struct bsdconv_codec *codec;
	struct data_rt *data_ptr;

	switch(r->state){
		case 0:
			switch(d){
				case 0x1B:
					r->state = 0x1B;
					this_phase->state.status = CONTINUE;
					return;
				default:
					switch(r->mode){
						case MODE_ASCII:
							this_phase->state.status = DEADEND;
							return;
						case MODE_JIS0201:
							codec = &r->jis0201;
							break;
						case MODE_JIS0208:
							codec = &r->jis0208;
							break;
						case MODE_JIS0212:
							codec = &r->jis0212;
							break;
					}
					offset_t offset = get_offset(codec, &r->cstate, d);
					r->cstate = read_state(codec, offset);
					if(r->cstate.status==MATCH){
						LISTCPY_ST(ins, this_phase->data_tail, r->cstate.data, codec->data_z);
						this_phase->state.status = NEXTPHASE;
						r->cstate = read_state(codec, 0);
					}else{
						this_phase->state.status = r->cstate.status;
						return;
					}
					break;
			}
			break;
			case 0x1B:
				switch(d){
					case 0x24:
						r->state = 0x1B24;
						this_phase->state.status = CONTINUE;
						return;
					case 0x28:
						r->state = 0x1B28;
						this_phase->state.status = CONTINUE;
						return;
					default:
						this_phase->state.status = DEADEND;
						return;
				}
				break;
			case 0x1B24:
				switch(d){
					case 0x40:
						r->state = 0;
						r->mode = MODE_JIS0208; //1978
						r->cstate = read_state(&r->jis0208, 0);
						this_phase->state.status = NEXTPHASE;
						return;
					case 0x42:
						r->state = 0;
						r->mode = MODE_JIS0208; //1983
						r->cstate = read_state(&r->jis0208, 0);
						this_phase->state.status = NEXTPHASE;
						return;
					default:
						this_phase->state.status = DEADEND;
						return;
				}
				break;
			case 0x1B28:
				switch(d){
					case 0x24:
						r->state = 0x1B2824;
						this_phase->state.status = CONTINUE;
						return;
					case 0x42:
						r->state = 0;
						r->mode = MODE_ASCII;
						this_phase->state.status = NEXTPHASE;
						return;
					case 0x49:
						r->state = 0;
						r->mode = MODE_JIS0201;
						r->cstate = read_state(&r->jis0201, 0);
						this_phase->state.status = NEXTPHASE;
						return;
					case 0x4A:
						r->state = 0;
						r->mode = MODE_JIS0201;
						r->cstate = read_state(&r->jis0201, 0);
						this_phase->state.status = NEXTPHASE;
						return;
					default:
						this_phase->state.status = DEADEND;
						return;
				}
				break;
			case 0x1B2824:
				switch(d){
					case 0x44:
						r->state = 0;
						r->mode = MODE_JIS0212;
						r->cstate = read_state(&r->jis0212, 0);
						this_phase->state.status = NEXTPHASE;
						break;
					default:
						this_phase->state.status = DEADEND;
						return;
				}
				break;
			default:
				this_phase->state.status = DEADEND;
				return;
	}

	return;
}
