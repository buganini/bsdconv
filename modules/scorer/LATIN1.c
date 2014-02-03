/*
 * Reference: http://en.wikipedia.org/wiki/Windows-1252
 */

#include "../../src/bsdconv.h"

static const struct uint32_range_with_score ranges[] = {
	{ 0x0, 0x80, 2 },
	{ 0x82, 0x8C, 2 },
	{ 0x8E, 0x8E, 2 },
	{ 0x91, 0x9C, 2 },
	{ 0x9E, 0xFF, 2 },
};

#include "unicode_range.c"
