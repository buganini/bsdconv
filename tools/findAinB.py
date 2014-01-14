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

total=0
inc=0
ninc=0

for k in la:
	total+=1
	if k in lb:
		inc+=1
		print("IN\t%s " % k)
		allnotin = False
	else:
		ninc+=1
		print("NOTIN\t%s " % k)
		allin = False

if allin:
	print("All In")
elif allnotin:
	print("All Not In")
else:
	print("Not All In")
print("Total: ", total)
print("In: ", inc)
print("Not In: ", ninc)
