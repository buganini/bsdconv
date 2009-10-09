#include <Python.h>
#include <bsdconv.h>

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
	{"info",	py_bsdconv_info,	METH_VARARGS,
		PyDoc_STR("conv() -> Return conversion info")},
	{"error",	py_bsdconv_error,	METH_VARARGS,
		PyDoc_STR("conv() -> Return error message")},
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
