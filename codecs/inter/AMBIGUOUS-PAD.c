/*
 * Some code and table come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
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

struct interval {
	int first;
	int last;
};

static const struct interval ambiguous[] = {
	{ 0x00A1, 0x00A1 }, { 0x00A4, 0x00A4 }, { 0x00A7, 0x00A8 },
	{ 0x00AA, 0x00AA }, { 0x00AE, 0x00AE }, { 0x00B0, 0x00B4 },
	{ 0x00B6, 0x00BA }, { 0x00BC, 0x00BF }, { 0x00C6, 0x00C6 },
	{ 0x00D0, 0x00D0 }, { 0x00D7, 0x00D8 }, { 0x00DE, 0x00E1 },
	{ 0x00E6, 0x00E6 }, { 0x00E8, 0x00EA }, { 0x00EC, 0x00ED },
	{ 0x00F0, 0x00F0 }, { 0x00F2, 0x00F3 }, { 0x00F7, 0x00FA },
	{ 0x00FC, 0x00FC }, { 0x00FE, 0x00FE }, { 0x0101, 0x0101 },
	{ 0x0111, 0x0111 }, { 0x0113, 0x0113 }, { 0x011B, 0x011B },
	{ 0x0126, 0x0127 }, { 0x012B, 0x012B }, { 0x0131, 0x0133 },
	{ 0x0138, 0x0138 }, { 0x013F, 0x0142 }, { 0x0144, 0x0144 },
	{ 0x0148, 0x014B }, { 0x014D, 0x014D }, { 0x0152, 0x0153 },
	{ 0x0166, 0x0167 }, { 0x016B, 0x016B }, { 0x01CE, 0x01CE },
	{ 0x01D0, 0x01D0 }, { 0x01D2, 0x01D2 }, { 0x01D4, 0x01D4 },
	{ 0x01D6, 0x01D6 }, { 0x01D8, 0x01D8 }, { 0x01DA, 0x01DA },
	{ 0x01DC, 0x01DC }, { 0x0251, 0x0251 }, { 0x0261, 0x0261 },
	{ 0x02C4, 0x02C4 }, { 0x02C7, 0x02C7 }, { 0x02C9, 0x02CB },
	{ 0x02CD, 0x02CD }, { 0x02D0, 0x02D0 }, { 0x02D8, 0x02DB },
	{ 0x02DD, 0x02DD }, { 0x02DF, 0x02DF }, { 0x0391, 0x03A1 },
	{ 0x03A3, 0x03A9 }, { 0x03B1, 0x03C1 }, { 0x03C3, 0x03C9 },
	{ 0x0401, 0x0401 }, { 0x0410, 0x044F }, { 0x0451, 0x0451 },
	{ 0x2010, 0x2010 }, { 0x2013, 0x2016 }, { 0x2018, 0x2019 },
	{ 0x201C, 0x201D }, { 0x2020, 0x2022 }, { 0x2024, 0x2027 },
	{ 0x2030, 0x2030 }, { 0x2032, 0x2033 }, { 0x2035, 0x2035 },
	{ 0x203B, 0x203B }, { 0x203E, 0x203E }, { 0x2074, 0x2074 },
	{ 0x207F, 0x207F }, { 0x2081, 0x2084 }, { 0x20AC, 0x20AC },
	{ 0x2103, 0x2103 }, { 0x2105, 0x2105 }, { 0x2109, 0x2109 },
	{ 0x2113, 0x2113 }, { 0x2116, 0x2116 }, { 0x2121, 0x2122 },
	{ 0x2126, 0x2126 }, { 0x212B, 0x212B }, { 0x2153, 0x2154 },
	{ 0x215B, 0x215E }, { 0x2160, 0x216B }, { 0x2170, 0x2179 },
	{ 0x2190, 0x2199 }, { 0x21B8, 0x21B9 }, { 0x21D2, 0x21D2 },
	{ 0x21D4, 0x21D4 }, { 0x21E7, 0x21E7 }, { 0x2200, 0x2200 },
	{ 0x2202, 0x2203 }, { 0x2207, 0x2208 }, { 0x220B, 0x220B },
	{ 0x220F, 0x220F }, { 0x2211, 0x2211 }, { 0x2215, 0x2215 },
	{ 0x221A, 0x221A }, { 0x221D, 0x2220 }, { 0x2223, 0x2223 },
	{ 0x2225, 0x2225 }, { 0x2227, 0x222C }, { 0x222E, 0x222E },
	{ 0x2234, 0x2237 }, { 0x223C, 0x223D }, { 0x2248, 0x2248 },
	{ 0x224C, 0x224C }, { 0x2252, 0x2252 }, { 0x2260, 0x2261 },
	{ 0x2264, 0x2267 }, { 0x226A, 0x226B }, { 0x226E, 0x226F },
	{ 0x2282, 0x2283 }, { 0x2286, 0x2287 }, { 0x2295, 0x2295 },
	{ 0x2299, 0x2299 }, { 0x22A5, 0x22A5 }, { 0x22BF, 0x22BF },
	{ 0x2312, 0x2312 }, { 0x2460, 0x24E9 }, { 0x24EB, 0x254B },
	{ 0x2550, 0x2573 }, { 0x2580, 0x258F }, { 0x2592, 0x2595 },
	{ 0x25A0, 0x25A1 }, { 0x25A3, 0x25A9 }, { 0x25B2, 0x25B3 },
	{ 0x25B6, 0x25B7 }, { 0x25BC, 0x25BD }, { 0x25C0, 0x25C1 },
	{ 0x25C6, 0x25C8 }, { 0x25CB, 0x25CB }, { 0x25CE, 0x25D1 },
	{ 0x25E2, 0x25E5 }, { 0x25EF, 0x25EF }, { 0x2605, 0x2606 },
	{ 0x2609, 0x2609 }, { 0x260E, 0x260F }, { 0x2614, 0x2615 },
	{ 0x261C, 0x261C }, { 0x261E, 0x261E }, { 0x2640, 0x2640 },
	{ 0x2642, 0x2642 }, { 0x2660, 0x2661 }, { 0x2663, 0x2665 },
	{ 0x2667, 0x266A }, { 0x266C, 0x266D }, { 0x266F, 0x266F },
	{ 0x273D, 0x273D }, { 0x2776, 0x277F }, { 0xE000, 0xF8FF },
	{ 0xFFFD, 0xFFFD }, { 0xF0000, 0xFFFFD }, { 0x100000, 0x10FFFD }
};

void callback(struct bsdconv_instance *ins){
	unsigned char *data;
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	data=this_phase->data->data;
	int pad;
	int max=sizeof(ambiguous) / sizeof(struct interval) - 1;
	int min = 0;
	int mid;
	char *space="\x01\x20";
	int ucs=0;

	if(data[0]==0x1){
		for(pad=1;pad<this_phase->data->len;++pad){
			ucs<<=8;
			ucs|=data[pad];
		}

		pad=0;
		if (ucs < ambiguous[0].first || ucs > ambiguous[max].last){
			pad=0;
		}else while (max >= min) {
				mid = (min + max) / 2;
				if (ucs > ambiguous[mid].last)
					min = mid + 1;
				else if (ucs < ambiguous[mid].first)
					max = mid - 1;
				else{
					pad=1;
					break;
				}
		}
		if(pad){
			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->len=2;
			this_phase->data_tail->data=space;
			this_phase->data_tail->flags=0;
		}
	}

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->data);
	this_phase->data->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;

	this_phase->state.status=NEXTPHASE;
	return;
}
