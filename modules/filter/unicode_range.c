/*
 * Some code come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

#include "../../src/bsdconv.h"

int cbfilter(struct data_rt *data){
	uint32_t ucs=0;
	int i;
	int max=sizeof(ranges) / sizeof(struct uint32_range) - 1;
	int min = 0;
	int mid;

	if(data->len<1 || UCP(data->data)[0]!=1){
		return 0;
	}

	for(i=1;i<data->len;++i){
		ucs<<=8;
		ucs|=UCP(data->data)[i];
	}

	if (ucs < ranges[0].first || ucs > ranges[max].last){
		//noop
	}else while (max >= min) {
		mid = (min + max) / 2;
		if (ucs > ranges[mid].last)
			min = mid + 1;
		else if (ucs < ranges[mid].first)
			max = mid - 1;
		else{
			return 1;
		}
	}

	return 0;
}
