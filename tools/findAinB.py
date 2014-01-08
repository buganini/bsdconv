import sys
import re

sep = re.compile(r"\s+")
stp = re.compile(r"^0[xX]")

fa = open(sys.argv[1])
fb = open(sys.argv[2])

la = {}
lb = {}

for f,l in ((fa, la), (fb, lb)):
	for ln in f:
		ln = ln.strip().upper()
		if ln == "":
			continue
		if ln.startswith("#"):
			continue
		a = sep.split(ln)
		p = stp.sub("", a[0])
		l[p]=1

allnotin = True
allin = True
for k in la:
	if k in lb:
		print(k)
		allnotin = False
	else:
		allin = False

if allin:
	print("All In")
else:
	print("Not All In")
if allnotin:
	print("All Not In")
