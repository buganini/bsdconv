# python codepage.py CPXXXX.TXT | sort > _CPXXXX.txt
import os
import sys
import re

def v(s):
    return int(s, 16)

def f1(s):
    s = s.strip()
    s = re.sub("^0x", "", s)
    return s

def f2(s):
    s = s.strip()
    s = re.sub("^0x", "", s)
    s = s.lstrip("0")
    if len(s) & 1:
        s = "0"+s
    return "01"+s

f = open(sys.argv[1])
for l in f:
    l = l.strip()
    if l=="":
        continue
    if l.startswith("#"):
        continue
    l = l.split("#")[0].strip()
    a = l.split("	")
    if len(a)<2:
        continue
    if v(a[0])<=0x7F and v(a[0])==v(a[1]):
        continue
    print f1(a[0])+"\t"+f2(a[1])
