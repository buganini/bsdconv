# -*- coding: utf-8 -*-
# python nfkc_gen.py '⁰¹²³'|sort|uniq
import sys
from bsdconv import Bsdconv

nfkc = Bsdconv("utf-8:nfkc:utf-8")
i = sys.argv[1].decode("utf-8")
for c in i:
	c = c.encode("utf-8")
	d = nfkc.conv(c)
	if c==d:
		continue
	print("{}\t{}".format(d, c))
