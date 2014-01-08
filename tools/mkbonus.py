#!/usr/bin/env python
import sys
import re
from bsdconv import Bsdconv

sc=Bsdconv("utf-8:zhtw:score#default:null")
bcv=Bsdconv("utf-8:zhtw:insert#after=002c:bsdconv-keyword,bsdconv")

sep=re.compile(r"\s+")

f=open(sys.argv[1])
for l in f:
	l = l.strip()
	if l == "":
		continue
	if l.startswith("#"):
		print(l)
	sc.counter_reset()
	a = sep.split(l)
	p = a[0]
	ln = len(p.decode("utf-8"))
	try:
		b = int(a[1])+1
	except:
		b = ln
	sc.conv(p)
	bonus = sc.counter("SCORE")
	if bonus < 5*ln:
		b += 5*ln - bonus
	p = bcv.conv(p).rstrip(",")
	print("%s\t?%02X" % (p, b))
