#!/usr/bin/env python3
import re
import sys
import requests
from bsdconv import Bsdconv

if len(sys.argv) != 2:
    print("Usage: {} outfile".format(sys.argv[0]))
    sys.exit(1)

outfile = sys.argv[1]

cv = Bsdconv("utf-8:split:bsdconv-keyword,bsdconv")

def bc(s):
    return cv.uconv(s).strip(",")

url = "https://raw.githubusercontent.com/apache/lucene-solr/master/lucene/analysis/common/src/java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilter.java"
java = requests.get(url).text
tks = re.findall(r"(?:case '(.*?)':|output\[outputPos\+\+\] = '(.*?)';|(break);)", java)

m = {}
f = []
t = ""
for tk in tks:
    if tk[2]=="break":
        m[t.encode("utf-8").decode("unicode_escape")] = f
        f = []
        t = ""
    elif tk[1]:
        t = t + tk[1]
    elif tk[0]:
        f.append(tk[0].encode("utf-8").decode("unicode_escape"))
    else:
        print("Unexpected Error")

l = []
for f in m:
    t = m[f]
    for c in t:
        print("{}\t{}".format(c, f))
        l.append((bc(c), bc(f)))

l.sort(key=lambda x: (len(x[0]), x[0]))

with open(outfile, "w") as out:
    out.write("# Derived from {}\n".format(url))
    for f, t in l:
        out.write("{}\t{}\n".format(f, t))
