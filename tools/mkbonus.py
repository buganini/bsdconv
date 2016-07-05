#!/usr/bin/env python

#mkbonus.py src_list char_list phrase_list

import sys
import re
from bsdconv import Bsdconv

clist=open(sys.argv[2], "w")
plist=open(sys.argv[3], "w")

sc=Bsdconv("utf-8:score#with=cjk:null")
bcv=Bsdconv("utf-8:insert#after=002c:bsdconv-keyword,bsdconv")
bcv_zhtw=Bsdconv("utf-8:zhtw:insert#after=002c:bsdconv-keyword,bsdconv")

sep=re.compile(r"\s+")

f=open(sys.argv[1])
for l in f:
	l = l.strip()
	if l == "":
		continue
	if l.startswith("#"):
		clist.write(l+"\n")
		plist.write(l+"\n")
	a = sep.split(l)
	p = a[0]
	ln = len(p.decode("utf-8"))
	if ln > 1:
		bonus = 6
		p = bcv_zhtw.conv(p).rstrip(",")
		of = plist
	else:
		try:
			bonus = int(a[1])
		except:
			bonus = 0
		sc.counter_reset()
		sc.conv(p)
		score = sc.counter("SCORE")
		if score < 5*ln:
			bonus += 5*ln - score
		if bonus == 0:
			continue
		p = bcv.conv(p).rstrip(",")
		of = clist
	of.write("%s\t?%02X,%s\n" % (p, bonus, p))

f.close()
clist.close()
plist.close()
