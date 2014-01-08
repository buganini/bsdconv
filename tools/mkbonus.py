#!/usr/bin/env python
import sys
import re
from bsdconv import Bsdconv

sc=Bsdconv("utf-8:score#default:null")
bcv=Bsdconv("utf-8:zhtw:insert#after=002c:bsdconv-keyword,bsdconv")

sep=re.compile(r"\s+")

f=open(sys.argv[1])
for l in f:
	l = l.strip()
	if l == "":
		continue
	if l.startswith("#"):
		print(l)
	a = sep.split(l)
	p = a[0]
	ln = len(p.decode("utf-8"))
	try:
		bonus = int(a[1])+1
	except:
		bonus = ln
	sc.counter_reset()
	sc.conv(p)
	score = sc.counter("SCORE")
	if score < 5*ln:
		bonus += 5*ln - score
	p = bcv.conv(p).rstrip(",")
	print("%s\t?%02X" % (p, bonus))
