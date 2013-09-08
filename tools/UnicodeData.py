import sys
import urllib

def bsdconv01(dt):
	dt=dt.lstrip("0")
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt


fccc=open("codecs/inter/_NF_CCC.inc", "w")
nfc=open("codecs/inter/_NFC.txt", "w")
nfd=open("codecs/inter/_NFD.txt", "w")
nfkc=open("codecs/inter/_NFKC.txt", "w")
nfkd=open("codecs/inter/_NFKD.txt", "w")
upper=open("codecs/inter/UPPER.txt", "w")
lower=open("codecs/inter/LOWER.txt", "w")

ccc_start=-1
ccc_end=-1
ccc_value=-1
nfcm={}
nfdm={}
nfkcm={}
nfkdm={}

fccc.write("/* Generated from {url}*/\n".format(url=sys.argv[1]));
for f in [nfc, nfd, nfkc, nfkd, upper, lower]:
	f.write("Source: {url}\n".format(url=sys.argv[1]))

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
				fccc.write("{0x%x, 0x%x, %d},\n" % (ccc_start, ccc_end, ccc_value))
			ccc_start=code_point
			ccc_end=code_point
			ccc_value=ccc
	if a[5]:
		dt=a[5].split(" ")
		compat=False
		if dt[0][0]=="<":
			mark=dt[0][1:-1]
			dt=dt[1:]
			compat=True
		dt=[bsdconv01(x) for x in dt]
		dt=",".join(dt)
		if compat:
			nfkd.write("{f}\t{t}\t#{c}\n".format(f=cp, t=dt, c=mark))
			nfkdm[cp]=dt
			nfkc.write("{f}\t{t}\t#{c}\n".format(f=dt, t=cp, c=mark))
			nfkcm[dt]=cp
		else:
			nfd.write("{f}\t{t}\n".format(f=cp, t=dt))
			nfdm[cp]=dt
			nfc.write("{f}\t{t}\n".format(f=dt, t=cp))
			nfcm[dt]=cp
	if a[12]:
		dt=bsdconv01(a[12])
		upper.write("{f}\t{t}\n".format(f=cp, t=dt))
	if a[13]:
		dt=bsdconv01(a[13])
		lower.write("{f}\t{t}\n".format(f=cp, t=dt))

fccc.write("{0x%x, 0x%x, %d},\n" % (ccc_start, ccc_end, ccc_value))
fccc.close()
nfc.close()
nfd.close()
nfkc.close()
nfkd.close()
upper.close()
lower.close()

def getMaxDepth(m):
	return max([getDepth(m, x) for x in m])

def getDepth(m, k):
	if k in m:
		return max([getDepth(m, x) for x in m[k].split(",")])+1
	else:
		return 0

print "NFC recursion depth:", getMaxDepth(nfcm)
print "NFKC recursion depth:", getMaxDepth(nfkcm)
