#include "../../src/bsdconv.h"

static const struct uint32_range ranges[] = {
	{ 0x30, 0x39 },
	{ 0x41, 0x5A },
	{ 0x61, 0x7A },
};

#include "unicode_range.c"
