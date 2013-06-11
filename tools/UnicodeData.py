import sys
import urllib

def bsdconv01(dt):
	dt=dt.lstrip("0")
	if len(dt) & 1:
		return "010"+dt
	else:
		return "01"+dt


nfc=open("../codecs/inter/NFC.txt", "w")
nfd=open("../codecs/inter/NFD.txt", "w")
nfkc=open("../codecs/inter/NFKC.txt", "w")
nfkd=open("../codecs/inter/NFKD.txt", "w")

for f in [nfc, nfd, nfkc, nfkd]:
	f.write("Source: {url}\n".format(url=sys.argv[1]))

ud=urllib.urlopen(sys.argv[1])
for l in ud:
	if not l.strip():
		continue
	a=l.split(";")
	cp=bsdconv01(a[0])
	if not a[5]:
		continue
	dt=a[5].split(" ")
	compat=False
	if dt[0][0]=="<":
		dt=dt[1:]
		compat=True
	dt=[bsdconv01(x) for x in dt]
	dt=",".join(dt)
	if compat:
		nfkd.write("{f}\t{t}\n".format(f=cp, t=dt))
		nfkc.write("{f}\t{t}\n".format(f=dt, t=cp))
	else:
		nfd.write("{f}\t{t}\n".format(f=cp, t=dt))
		nfc.write("{f}\t{t}\n".format(f=dt, t=cp))

nfc.close()
nfd.close()
nfkc.close()
nfkd.close()
