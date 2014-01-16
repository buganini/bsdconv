/*
 * Some code come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 *
 * Copyright (c) 2012-2014 Kuan-Chung Chiu <buganini@gmail.com>
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
