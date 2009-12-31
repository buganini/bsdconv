/*
 * Copyright (c) 2009,2010 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <Python.h>
#include <bsdconv.h>

#define IBUFLEN 1024
#define OBUFLEN 1024

PyDoc_STRVAR(bsdconv_create_doc,
"create(c)\n\
\n\
Create bsdconv instance.");

static PyObject *
py_bsdconv_create(PyObject *self, PyObject *args)
{
	char *c;
	struct bsdconv_instance *r;

	if (!PyArg_ParseTuple(args, "s", &c))
		return NULL;
	r=bsdconv_create(c);
	if(r==NULL){
		Py_INCREF(Py_None);
		return Py_None;
	}
	return Py_BuildValue("k",r);
}

PyDoc_STRVAR(bsdconv_destroy_doc,
"destroy(p)\n\
\n\
Destroy bsdconv instance.");

static PyObject *
py_bsdconv_destroy(PyObject *self, PyObject *args)
{
	unsigned long k;
	struct bsdconv_instance *r;
	if (!PyArg_ParseTuple(args, "k", &k))
		return NULL;
	r=(struct bsdconv_instance *) k;
	bsdconv_destroy(r);

	Py_INCREF(Py_True);
	return Py_True;
}

PyDoc_STRVAR(bsdconv_conv_doc,
"conv(p,s)\n\
\n\
Perform conversion.");

static PyObject *
py_bsdconv_conv(PyObject *self, PyObject *args)
{
	unsigned long k;
	static PyObject *r;
	struct bsdconv_instance *p;
	char *s;
	int l;
	if (!PyArg_ParseTuple(args, "kz#", &k,&s,&l))
		return NULL;
	p=(struct bsdconv_instance *) k;
	p->mode=BSDCONV_CC;
	p->feed=s;
	p->feed_len=l;
	bsdconv_init(p);
	bsdconv(p);
	r=Py_BuildValue("s#",p->back, p->back_len);
	free(p->back);
	return r;
}

PyDoc_STRVAR(bsdconv_conv_file_doc,
"conv_file(p,s,s)\n\
\n\
Perform conversion with given filename.");

static PyObject *
py_bsdconv_conv_file(PyObject *self, PyObject *args)
{
	unsigned long k;
	struct bsdconv_instance *p;
	char *s1, *s2;
	FILE *inf, *otf;
	unsigned char in[IBUFLEN], out[OBUFLEN];
	int r;

	if (!PyArg_ParseTuple(args, "kss", &k,&s1,&s2))
		return NULL;
	p=(struct bsdconv_instance *) k;
	inf=fopen(s1,"r");
	if(!inf){
		Py_INCREF(Py_None);
		return Py_None;
	}
	otf=fopen(s2,"w");
	if(!otf){
		Py_INCREF(Py_None);
		return Py_None;
	}
	p->in_buf=in;
	p->in_len=IBUFLEN;
	p->out_buf=out;
	p->out_len=OBUFLEN;
	p->mode=BSDCONV_BB;
	bsdconv_init(p);
	do{
		if(p->feed_len) p->feed_len=fread(p->feed, 1, p->feed_len, inf);
		r=bsdconv(p);
		if(p->back_len)fwrite(p->back, 1, p->back_len, otf);
	}while(r);

	fclose(inf);
	fclose(otf);

	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
py_bsdconv_info(PyObject *self, PyObject *args)
{
	unsigned long k;
	static PyObject *r;
	struct bsdconv_instance *p;
	if (!PyArg_ParseTuple(args, "k", &k))
		return NULL;
	p=(struct bsdconv_instance *) k;
	r=Py_BuildValue("{sisi}","ierr",p->ierr,"oerr",p->oerr);
	return r;
}

static PyObject *
py_bsdconv_error(PyObject *self, PyObject *args)
{
	static PyObject *r;
	char *s;
	s=bsdconv_error();
	r=Py_BuildValue("s",s);
	free(s);
	return r;
}

static PyMethodDef bsdconv_methods[] = {
	{"create",	py_bsdconv_create,	METH_VARARGS,
		PyDoc_STR("create() -> Create bsdconv instance")},
	{"destroy",	py_bsdconv_destroy,	METH_VARARGS,
		PyDoc_STR("destroy() -> Destroy bsdconv instance")},
	{"conv",	py_bsdconv_conv,	METH_VARARGS,
		PyDoc_STR("conv() -> Perform conversion")},
	{"conv_file",	py_bsdconv_conv_file,	METH_VARARGS,
		PyDoc_STR("conv_file() -> Perform conversion with given filename")},
	{"info",	py_bsdconv_info,	METH_VARARGS,
		PyDoc_STR("info() -> Return conversion info")},
	{"error",	py_bsdconv_error,	METH_VARARGS,
		PyDoc_STR("error() -> Return error message")},
	{NULL,		NULL}		/* sentinel */
};

PyDoc_STRVAR(module_doc,
"BSD licensed charset/encoding converter library");

/* Initialization function for the module (*must* be called initxx) */

PyMODINIT_FUNC
initbsdconv(void)
{
	PyObject *m;
	m = Py_InitModule3("bsdconv", bsdconv_methods, module_doc);
	if (m == NULL)
		return;
}
