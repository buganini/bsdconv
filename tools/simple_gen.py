# simple_gen.py phase_type from_column to_column file
import sys
import re

def bsdconv01(dt):
	dt=dt.strip().lstrip("0").upper()
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt

def raw(dt):
	return dt

pt = sys.argv[1].upper()
if pt == "FROM":
	ff = raw
	tf = bsdconv01
elif pt == "INTER":
	ff = bsdconv01
	tf = bsdconv01
else:
	ff = bsdconv01
	tf = raw

stp = re.compile(r"^(U\+|0X)")
sep = re.compile(r"\s+")
vld = re.compile(r"^[a-fA-F0-9,]+$")

from_column = int(sys.argv[2])
to_column = int(sys.argv[3])

f=open(sys.argv[4])
for l in f:
	l = l.strip().upper()
	if l == "":
		continue
	if l.startswith("#"):
		continue
	a = sep.split(l)
	fr = stp.sub("", a[from_column])
	to = stp.sub("", a[to_column])
	if not vld.match(fr):
		continue
	if not vld.match(to):
		continue

	print("%s\t%s" % (ff(fr), tf(to)))
