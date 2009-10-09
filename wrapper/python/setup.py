#!/usr/bin/env python

from distutils.core import setup,Extension

setup(name='bsdconv',
      version='3.5',
      description='bsdconv library',
      author='Buganini',
      author_email='buganini@gmail.com',
      url='http://github.com/buganini/bsdconv',
      ext_modules=[Extension('bsdconv', ['bsdconv.c'],
              include_dirs=['/usr/local/include'],
              libraries=['bsdconv'],
              library_dirs=['/usr/local/lib'])],
     )

