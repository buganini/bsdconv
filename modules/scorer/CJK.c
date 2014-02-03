/*
 * Reference: http://blog.oasisfeng.com/2006/10/19/full-cjk-unicode-range/
 */

#include "../../src/bsdconv.h"

static const struct uint32_range_with_score ranges[] = {
	{ 0x0, 0x7F, 4 },	//ASCII
	{ 0x3000, 0x303F, 4 },	//CJK punctuation
	{ 0x3040, 0x309F, 5 },	//Japanese hiragana
	{ 0x30A0, 0x30FF, 5 },	//Japanese katakana
	{ 0x3100, 0x312F, 4 },	//Chinese Bopomofo
	{ 0x3400, 0x4DB5, 3 },	//CJK Unified Ideographs Extension A	;Unicode3.0
	{ 0x4E00, 0x6FFF, 5 },	//CJK Unified Ideographs	;Unicode 1.1	;HF
	{ 0x7000, 0x9FA5, 4 },	//CJK Unified Ideographs	;Unicode 1.1	;LF
	{ 0x9FA6, 0x9FBB, 3 },	//CJK Unified Ideographs	;Unicode 4.1
	{ 0xAC00, 0xD7AF, 3 },	//Korean word
	{ 0xF900, 0xFA2D, 4 },	//CJK Compatibility Ideographs	;Unicode 1.1
	{ 0xFA30, 0xFA6A, 4 },	//CJK Compatibility Ideographs	;Unicode 3.2
	{ 0xFA70, 0xFAD9, 2 },	//CJK Compatibility Ideographs	;Unicode 4.1
	{ 0xFF00, 0xFFEF, 3},	//Fullwidth ASCII, punctuation, Japanese, Korean
	{ 0x20000, 0x2A6D6, 1 },//CJK Unified Ideographs Extension B	;Unicode 3.1
	{ 0x2F800, 0x2FA1D, 1 },//CJK Compatibility Supplement	;Unicode 3.1
};

#include "unicode_range.c"
