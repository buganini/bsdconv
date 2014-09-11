import sys
import re
import os

def bsdconv01(dt):
	dt=dt.strip().lstrip("0").upper()
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt


f_ccc=open("modules/inter/_NF-CCC.h", "w")
f_nfd=open("modules/inter/_NFD.txt", "w")
f_nfkd=open("modules/inter/_NFKD.txt", "w")
f_nfc=open("modules/inter/_NFC-MAP.txt", "w")
f_upper=open("modules/inter/UPPER.txt", "w")
f_lower=open("modules/inter/LOWER.txt", "w")
f_casefold=open("modules/inter/CASEFOLD.txt", "w")

ccc_start=-1
ccc_end=-1
ccc_value=0
t_ccc={}
m_nfd={}
m_nfd_raw={}
m_nfkd={}

m_url={}
f_map=open("tmp/map.txt")
for l in f_map:
	l=l.strip().split("\t")
	if len(l)==2:
		m_url[l[0]]=l[1]

f_ccc.write("/* Generated from {url}*/\n".format(url=m_url["UnicodeData.txt"]));
for f in [f_nfc, f_nfd, f_nfkd, f_upper, f_lower]:
	f.write("Source: {url}\n".format(url=m_url["UnicodeData.txt"]))

f_upper.write("Source: {url}\n".format(url=m_url["SpecialCasing.txt"]))
f_lower.write("Source: {url}\n".format(url=m_url["SpecialCasing.txt"]))
f_casefold.write("Source: {url}\n".format(url=m_url["CaseFolding.txt"]))

f_ccc.write("""
	struct ccc_interval {
	int beg;
	int end;
	int ccc;
};

static const struct ccc_interval ccc_table[] = {
""");

def lookup(l,m):
	ret=[]
	for e in l:
		if e in m:
			ret.extend(lookup(m[e], m))
		else:
			ret.append(e)
	return ret

def nf_order(l):
	r=[]
	b=0
	e=0
	for i in range(len(l)):
		if l[i] in t_ccc:
			if b==0:
				b=i
				e=i
			else:
				e=i
		else:
			if b!=e:
				r.append((b,e))
			b=0
			e=0
	if b!=e:
		r.append((b,e))
	for b,e in r:
		a=sorted(l[b:e+1], key=lambda x:t_ccc[x])
		for i in range(b,e+1):
			l[i]=a[i-b]
	return l

def in_range(s,rs):
	s=int(s[2:], 16)
	for r0,r1 in rs:
		r0=int(r0[2:], 16)
		r1=int(r1[2:], 16)
		if s>=r0 and s<=r1:
			return True
	return False

l_nfd=[]
l_nfkd=[]
ud=open("tmp/UnicodeData.txt")
for l in ud:
	if not l.strip():
		continue
	a=l.split(";")
	cp=bsdconv01(a[0])
	code_point=int(a[0], 16)
	if a[3]!="0":
		ccc=int(a[3])
		if ccc:
			t_ccc[cp]=ccc
		if ccc==ccc_value and code_point==ccc_end+1:
			ccc_end=code_point
		else:
			if ccc_value!=0:
				f_ccc.write("{0x%x, 0x%x, %d},\n" % (ccc_start, ccc_end, ccc_value))
			ccc_start=code_point
			ccc_end=code_point
			ccc_value=ccc
	if a[5]:
		dt=a[5].split(" ")
		compat=False
		if dt[0][0]=="<":
			tag=dt[0][1:-1]
			dt=dt[1:]
			compat=True
		dt=[bsdconv01(x) for x in dt]
		if compat:
			l_nfkd.append((cp,tag))
			m_nfkd[cp]=dt
		else:
			l_nfkd.append((cp,"canonical"))
			m_nfkd[cp]=dt
			l_nfd.append(cp)
			m_nfd[cp]=dt
			m_nfd_raw[cp]=dt
	if a[12]:
		dt=bsdconv01(a[12])
		f_upper.write("{f}\t{t}\n".format(f=cp, t=dt))
	if a[13]:
		dt=bsdconv01(a[13])
		f_lower.write("{f}\t{t}\n".format(f=cp, t=dt))

f_ccc.write("{0x%x, 0x%x, %d},\n" % (ccc_start, ccc_end, ccc_value))
f_ccc.write("};\n")
f_ccc.close()

sc=open("tmp/SpecialCasing.txt")
for l in sc:
	l=l.strip()
	if not l:
		continue
	if l[0] in "#":
		continue
	d,c=l.split("#")
	d=d.split(";")
	code=",".join([bsdconv01(x) for x in d[0].strip().split(" ")])
	lower=",".join([bsdconv01(x) for x in d[1].strip().split(" ")])
	title=",".join([bsdconv01(x) for x in d[2].strip().split(" ")])
	upper=",".join([bsdconv01(x) for x in d[3].strip().split(" ")])
	cond=d[4].strip()
	if cond=="":
		if code!=upper:
			f_upper.write("{f}\t{t}\n".format(f=code, t=upper))
		if code!=lower:
			f_lower.write("{f}\t{t}\n".format(f=code, t=lower))


f_upper.close()
f_lower.close()

l_fce=[]
dnp=open("tmp/DerivedNormalizationProps.txt")
for l in dnp:
	l=l.strip()
	if not l:
		continue
	if l[0] in "#":
		continue
	a=l.split(";")
	if not a[1].strip().startswith("Full_Composition_Exclusion"):
		continue
	r=a[0].strip().split("..")
	if len(r)==1:
		r.append(r[0])
	l_fce.append((bsdconv01(r[0]),bsdconv01(r[1])))

for cp in l_nfd:
	d=nf_order(lookup(m_nfd[cp], m_nfd))
	m_nfd[cp]=d
	f_nfd.write("{f}\t{t}\n".format(f=cp, t=",".join(d)))
f_nfd.close()

for cp,tag in l_nfkd:
	d=nf_order(lookup(m_nfkd[cp], m_nfkd))
	m_nfkd[cp]=d
	f_nfkd.write("{f}\t{t}\t#{c}\n".format(f=cp, t=",".join(d), c=tag))
f_nfkd.close()

for cp in l_nfd:
	if in_range(cp, l_fce):
		continue
	l=m_nfd_raw[cp]
	f_nfc.write("{f}\t{t}\n".format(f=",".join(l), t=cp))
f_nfc.close()

cf=open("tmp/CaseFolding.txt")
for l in cf:
	l=l.strip()
	if not l:
		continue
	if l[0] in "#":
		continue
	csm, name = l.split("#")
	code, status, mapping, null = csm.split(";")
	code = bsdconv01(code)
	status=status.strip()
	mapping = ",".join([bsdconv01(x) for x in mapping.strip().split(" ")])
	f_casefold.write("{f}\t{t}\n".format(f=code, t=mapping))
f_casefold.close()


# Blocks

blocks={
	"^.*Arabian$": "ARABIC",
	"^Arabic.*$": "ARABIC",
	"^Armenian$": "ARMENIAN",
	"^.*Arrows.*$": "ARROWS",
	"^Bopomofo.*$": "CJK",
	"^Braille.*$": "BRAILLE",
	"^Cherokee$": "CHEROKEE",
	"^.*CJK.*$": "CJK",
	"^Cuneiform.*$": "CUNEIFORM",
	"^Currency.*$": "CURRENCY",
	"^Cyrillic.*$": "CYRILLIC",
	"^Devanagari.*$": "DEVANAGARI",
	"^Egyptian.*$": "EGYPTIAN",
	"^Emoticons$": "EMOTICON",
	"^Ethiopic.*$": "ETHIOPIC",
	"^Georgian.*$": "GEORGIAN",
	"^.*Greek.*$": "GREEK",
	"^Hangul.*$": ["HANGUL", "CJK"],
	"^Hebrew$": "HEBREW",
	"^Hiragana$": ["HIRAGANA", "CJK"],
	"^Ideographic Description Characters*": "CJK",
	"^IPA.*$": ["IPA", "PHONETIC"],
	"^Javanese$": "JAVANESE",
	"^Katakana.*$": ["KATAKANA", "CJK"],
	"^Kana .*$": "CJK",
	"^Kanbun.*$": "CJK",
	"^Kangxi Radicals$": "CJK",
	"^Kannada$": "KANNADA",
	"^Khmer.*$": "KHMER",
	"^Lao$": "LAO",
	"^.*Latin.*$": "LATIN",
	"^Miao$": "MIAO",
	"^Mahjong.*$": "MAHJONG",
	"^Malayalam$": "MALAYALAM",
	"^.*Mathematical.*$": "MATH",
	"^Mongolian$": "MONGOLIAN",
	"^.*Musical.*$": "MUSIC",
	"^Myanmar.*$": "MYANMAR",
	"^Phonetic.*$": "PHONETIC",
	"^.*Private Use Area.*$": "PUA",
	"^.*Punctuation.*$": "PUNCTUATION",
	"^Samaritan$": "SAMARITAN",
	"^Sinhala.*$": "SINHALA",
	"^Sundanese.*$": "SUNDANESE",
	"^Syriac$": "SYRIAC",
	"^Tagalog$": "TAGALOG",
	"^Tai Xuan Jing.*$": "CJK",
	"^Tamil$": "TAMIL",
	"^Telugu$": "TELUGU",
	"^Thai$": "THAI",
	"^Tibetan$": "TIBETAN",
	"^Tifinagh$": "TIFINAGH",
	"^Yi .*$": ["YI", "CJK"],
	"^Yijing.*$": "CJK",
}

m={}
blk=open("tmp/Blocks.txt")
for l in blk:
	l=l.strip()
	if l=="" or l[0]=="#":
		continue
	r, d = l.split(";")
	d=d.strip()
	cl=[]
	for pt in blocks:
		if re.match(pt, d):
			c=blocks[pt]
			if type(c)==list:
				cl.extend(c)
			else:
				cl.append(c)
	print r, d, cl
	for c in cl:
		if c not in m:
			m[c]=open(os.path.join("modules/filter", c+".c"), "w")
			m[c].write("/*\n"
			" * Generated from: "+m_url["Blocks.txt"]+"\n"
			" */\n"
			"\n"
			"#include \"../../src/bsdconv.h\"\n"
			"\n"
			"static const struct uint32_range ranges[] = {\n"
			)
		b,e=r.split("..")
		m[c].write("\t{{ 0x{beg}, 0x{end} }}, // {desc}\n".format(beg=b, end=e, desc=d))

for c in m:
	m[c].write("};\n"
	"#include \"unicode_range.c\"\n")
	m[c].close()
