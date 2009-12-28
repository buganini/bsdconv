
#argv[1] CNS_component_word_yyyymmdd.txt
#argv[2] CNS_component_yyyymmdd.txt
import os,sys

def p(s):
	if(len(s)%2):
		return '0'+s;
	return s;

def tw(s):
	try:
		r=w[s];
	except:
		r=s;
	return r;

w={};
fi=open(sys.argv[1],'rU')
for l in fi:
	a,b,c=l.strip().split('\t');
	w[int(a)]=int(b);
fi.close()

fi=open(sys.argv[2],'rU')
for l in fi:
	a,b,c=l.strip().split('\t');
	a=p(a)
	b=p(b)
	cns="02%s%s" % (a,b)
	comps=c.strip(';').split(';');
	for comp in comps:
		com=','.join(["04"+p("%X" % tw(int(x.strip()))) for x in comp.strip(',').split(',')]);
		print "%s\t%s" % (cns,com)
