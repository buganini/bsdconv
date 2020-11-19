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

entries = []
for f in m:
    t = m[f]
    for c in t:
        print("{}\t{}".format(c, f))
        entries.append((bc(c), bc(f)))

codepoints = [
    ["𝐀", "𝐙"], # BOLD CAPITAL
    ["𝐚", "𝐳"], # BOLD SMALL
    ["𝑨", "𝒁"], # BOLD ITALIC CAPITAL
    ["𝒂", "𝒛"], # BOLD ITALIC SMALL
    ["𝓐", "𝓩"], # BOLD SCRIPT CAPITAL
    ["𝓪", "𝔃"], # BOLD SCRIPT SMALL
    ["𝕬", "𝖅"], # BOLD FRAKTUR CAPITAL
    ["𝖆", "𝖟"], # BOLD FRAKTUR SMALL
    ["𝗔", "𝗭"], # SANS-SERIF BOLD CAPITAL
    ["𝗮", "𝘇"], # SANS-SERIF BOLD SMALL
    ["𝘼", "𝙕"], # SANS-SERIF BOLD ITALIC CAPITAL
    ["𝙖", "𝙯"], # SANS-SERIF BOLD ITALIC SMALL
    ["𝟎", "𝟗"], # BOLD DIGIT
    ["𝟬", "𝟵"], # SANS-SERIF BOLD DIGIT
]

import unicodedata

dm = {
    "ZERO":"0",
    "ONE":"1",
    "TWO":"2",
    "THREE":"3",
    "FOUR":"4",
    "FIVE":"5",
    "SIX":"6",
    "SEVEN":"7",
    "EIGHT":"8",
    "NINE":"9",
}

for b,e in codepoints:
    for i in range(ord(b), ord(e)+1):
        c = chr(i)
        name = unicodedata.name(c)
        tks = name.split(" ")
        m = tks[-2]
        l = tks[-1]
        if m=="CAPITAL":
            l = l.upper()
        elif m=="SMALL":
            l = l.lower()
        elif m=="DIGIT":
            l = dm[l]
        print("{}\t{}".format(c, l))
        entries.append((bc(c), bc(l)))

entries.sort(key=lambda x: (len(x[0]), x[0]))

with open(outfile, "w") as out:
    out.write("# Partially derived from {}\n".format(url))
    for f, t in entries:
        out.write("{}\t{}\n".format(f, t))
