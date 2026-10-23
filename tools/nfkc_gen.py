#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# python3 nfkc_gen.py '⁰¹²³'|sort|uniq
import sys
from bsdconv import Bsdconv

nfkc = Bsdconv("utf-8:nfkc:utf-8")
i = sys.argv[1]
for c in i:
	d = nfkc.uconv(c)
	if c==d:
		continue
	print("{}\t{}".format(d, c))
