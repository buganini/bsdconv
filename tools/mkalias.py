#!/usr/bin/env python3
import os,sys,re;

def f(c):
	s="%X" % ord(c)
	if len(s)%2:
		r='010%s' % s;
	else:
		r='01%s' % s;
	return r;

fi=open(sys.argv[1],'r', encoding="utf-8");
fo=open(sys.argv[2],'w', encoding="utf-8");

for l in fi:
	if l.find('\t')!=-1:
		a,b=re.split('\t+',l.strip());
		a=','.join([f(c) for c in a]);
		b=','.join([f(c) for c in b]);
		fo.write("%s\t%s\n" % (a,b));

fi.close();
fo.close();

