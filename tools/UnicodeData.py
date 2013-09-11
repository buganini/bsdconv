import sys
import urllib

def bsdconv01(dt):
	dt=dt.lstrip("0").upper()
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt


f_ccc=open("codecs/inter/_NF_CCC.inc", "w")
f_nfd=open("codecs/inter/_NFD.txt", "w")
f_nfkd=open("codecs/inter/_NFKD.txt", "w")
f_nfc=open("codecs/inter/_NFC.txt", "w")
f_upper=open("codecs/inter/UPPER.txt", "w")
f_lower=open("codecs/inter/LOWER.txt", "w")

ccc_start=-1
ccc_end=-1
ccc_value=0
t_ccc={}
m_nfd={}
m_nfkd={}

f_ccc.write("/* Generated from {url}*/\n".format(url=sys.argv[1]));
for f in [f_nfc, f_nfd, f_nfkc, f_nfkd, f_upper, f_lower]:
	f.write("Source: {url}\n".format(url=sys.argv[1]))

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
	if a[12]:
		dt=bsdconv01(a[12])
		f_upper.write("{f}\t{t}\n".format(f=cp, t=dt))
	if a[13]:
		dt=bsdconv01(a[13])
		f_lower.write("{f}\t{t}\n".format(f=cp, t=dt))

f_ccc.write("{0x%x, 0x%x, %d},\n" % (ccc_start, ccc_end, ccc_value))
f_ccc.close()

f_upper.close()
f_lower.close()

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
	l=m_nfd[cp]
	f_nfc.write("{f}\t{t}\n".format(f=",".join(l), t=cp))
f_nfc.close()
