/*
 * Generated from: ftp://ftp.unicode.org/Public/9.0.0/ucd/Blocks.txt
 */

#include "../../src/bsdconv.h"

static const struct uint32_range ranges[] = {
	{ 0xE000, 0xF8FF }, // Private Use Area
	{ 0xF0000, 0xFFFFF }, // Supplementary Private Use Area-A
	{ 0x100000, 0x10FFFF }, // Supplementary Private Use Area-B
};
#include "unicode_range.c"
