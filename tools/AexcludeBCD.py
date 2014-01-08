# AexcludeBCD.py A B [C...]

import sys
import re

sep = re.compile(r"\s+")

excl={}

for fn in sys.argv[2:]:
	f = open(fn)
	for l in f:
		l = l.strip()
		if l == "":
			continue
		if l.startswith("#"):
			continue
		a = sep.split(l)
		p = a[0].upper()
		excl[p] = 1
	f.close()

f = open(sys.argv[1])
for l in f:
	l2 = l.strip()
	if l2 == "":
		sys.stdout.write(l)
		continue
	if l2.startswith("#"):
		sys.stdout.write(l)
		continue
	a = sep.split(l2)
	p = a[0].upper()
	if p not in excl:
		sys.stdout.write(l)
f.close()
