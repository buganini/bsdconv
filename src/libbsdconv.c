#include <stdlib.h>
#include <bsdconv.h>

struct bsdconv_codec_t {
	desc
	fd
	mmap
}

struct bsdconv_t {
	from
	inter
	to
};

bsdconv_create(){

}

bsdconv_destroy(){

}

bsd_conv(const char *inbuf, size_t *inlen, char *outbuf, size_t *outlen){
	//from
	while(form_ptr!=from_ptr_end && from_continue){
		if(from_reset){
			from_reset=0;
			from_state=0;
			inter_data='?';
		}
		from_data=*from_ptr++;
		from_state=from_map[from_index][from_state]->sub[from_data];
		if(from_state->len){
			inter_data=from_state->data;
		}else if(from_index==from_index_end){
			from_ptr++;
			from_continue=0;
			break;
		}else{
			from_index++;
			from_reset=1;
		}
	}

	//inter
	to_data=empty;
	for(i=0;i<data_len;i++){
		inter_state=inter_map[inter_index][inter_state]->sub[inter_data];
		if(inter_state->len){
			has_match=1;
		}else if(has_match){
			has_match=0;
			to_date=last_match;
			inter_index=0;
			inter_state=0;
		}else if(inter_index==index_index_end){
			to_date=inter_data;
		}else{
			inter_index++;
		}
	}

	//to
	while(to_data && to_continue){
		to_state=to_map[to_index][to_state]->sub[to_data];
		if(to_state->len){
			out_data=
		}else if(has_match){
		}elseif (to_index==to_index_end){
			
		}else{
		
		}
	}
}

