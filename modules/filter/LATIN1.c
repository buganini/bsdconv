/*
 * Reference: http://en.wikipedia.org/wiki/Windows-1252
 */

#include "../../src/bsdconv.h"

static const struct uint32_range ranges[] = {
	{ 0x0, 0x80 },
	{ 0x82, 0x8C },
	{ 0x8E, 0x8E },
	{ 0x91, 0x9C },
	{ 0x9E, 0xFF },
};

#include "unicode_range.c"
