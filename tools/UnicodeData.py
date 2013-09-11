import sys
import urllib

def bsdconv01(dt):
	dt=dt.lstrip("0")
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt


f_ccc=open("codecs/inter/_NF_CCC.inc", "w")
f_nfd=open("codecs/inter/_NFD.txt", "w")
f_nfkd=open("codecs/inter/_NFKD.txt", "w")
nfc=open("codecs/inter/_NFC.txt", "w")
nfkc=open("codecs/inter/_NFKC.txt", "w")
upper=open("codecs/inter/UPPER.txt", "w")
lower=open("codecs/inter/LOWER.txt", "w")

ccc_start=-1
ccc_end=-1
ccc_value=-1
m_nfd={}
m_nfkd={}

f_ccc.write("/* Generated from {url}*/\n".format(url=sys.argv[1]));
for f in [nfc, f_nfd, nfkc, f_nfkd, upper, lower]:
	f.write("Source: {url}\n".format(url=sys.argv[1]))

def lookup(l,m):
	ret=[]
	for e in l:
		if e in m:
			ret.extend(lookup(m[e], m))
		else:
			ret.append(e)
	return ret

l_nfd=[]
l_nfkd=[]
ud=urllib.urlopen(sys.argv[1])
for l in ud:
	if not l.strip():
		continue
	a=l.split(";")
	cp=bsdconv01(a[0])
	code_point=int(a[0], 16)
	if a[3]!="0":
		ccc=int(a[3])
		if ccc==ccc_value and code_point==ccc_end+1:
			ccc_end=code_point
		else:
			if ccc_value!=-1:
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
	if a[12]:
		dt=bsdconv01(a[12])
		upper.write("{f}\t{t}\n".format(f=cp, t=dt))
	if a[13]:
		dt=bsdconv01(a[13])
		lower.write("{f}\t{t}\n".format(f=cp, t=dt))

f_ccc.write("{0x%x, 0x%x, %d},\n" % (ccc_start, ccc_end, ccc_value))
f_ccc.close()

for cp in l_nfd:
	f_nfd.write("{f}\t{t}\n".format(f=cp, t=",".join(lookup(m_nfd[cp], m_nfd))))
f_nfd.close()

for cp,tag in l_nfkd:
	f_nfkd.write("{f}\t{t}\t#{c}\n".format(f=cp, t=",".join(lookup(m_nfkd[cp], m_nfd)), c=tag))
f_nfkd.close()

nfc.close()
nfkc.close()
upper.close()
lower.close()
