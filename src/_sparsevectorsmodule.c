#include "Python.h"
#include "structmember.h"

#ifdef STDC_HEADERS
#include <stddef.h>
#else
#include <sys/types.h>          /* For size_t */
#endif

// TODO: only allow float as element type?

#define BLOCKLEN 64
#define CENTER ((BLOCKLEN - 1) / 2)

typedef struct {
    PyDictObject dict;
} sparsevecobject;

static PyTypeObject sparsevec_type; /* Forward */

PyDoc_STRVAR(sparsevec_missing_doc,
"__missing__(key) # returns value 0.0 but does NOT insert it!\n\
");

// TODO: this should not actually add anything to the dictionary!!
static PyObject *
sparsevec_missing(sparsevecobject *dd, PyObject *key)
{
    PyObject *value;
    value = PyFloat_FromDouble(0.0);
    Py_INCREF(value);
    return value;
}


static double dot(sparsevecobject *self, sparsevecobject *other)
{
  Py_ssize_t i = 0;
  PyObject *key;
  PyObject *value;

  double sum = 0;

  while (PyDict_Next((PyObject *)self, &i, &key, &value)) {
    // TODO: reference counting???
    // We call the PyDict_GetItem method directly here, not our own? So we still
    // get a NULL and not a 0.0 here?
    PyFloatObject *val = (PyFloatObject *)PyDict_GetItem((PyObject *)other, (PyObject *)key);
    if (val != NULL) {
        sum += PyFloat_AS_DOUBLE((PyFloatObject *)value) * PyFloat_AS_DOUBLE(val);
    }
  }

  return sum;
}


static PyObject *
sparsevec_dot(sparsevecobject *self, sparsevecobject *other)
{
    double sum = 0.0;

    Py_ssize_t len_self = PyObject_Size((PyObject*)self);
    Py_ssize_t len_other = PyObject_Size((PyObject*)other);

    if(len_self == 0 || len_other == 0) {
        PyObject *value = PyFloat_FromDouble(0.0);
        Py_INCREF(value); // ???
        return value;
    }
    sum = len_self < len_other ? dot(self, other) : dot(other, self);
    PyObject *value =  PyFloat_FromDouble(sum);
    Py_INCREF(value); // ???
    return value;
}



static PyObject *
sparsevec_iaddc(sparsevecobject *self, PyObject *args)
{
    Py_ssize_t i = 0;
    PyObject *key;
    PyObject *value;

    PyObject *other;
    double c_value = 1;

    // TODO: reference counting???
    PyArg_ParseTuple(args, "O|d", &other, &c_value);
    while (PyDict_Next((PyObject *)other, &i, &key, &value)) {
      // TODO: check if other and this are both floats.
      // if value is not a float, complain
      // TODO: maybe check for subtype as well using PyType_IsSubtype(value->ob_type,&PyFloat_Type)?

      if (!PyFloat_Check(value)) {
        PyErr_Format(PyExc_TypeError,"SparseVector must contain floats not: %s\n",Py_TYPE(value)->tp_name);
        Py_RETURN_NONE;
      }

      double otherval = PyFloat_AS_DOUBLE((PyFloatObject *)value);
      PyFloatObject *item = (PyFloatObject *)PyDict_GetItem((PyObject *)self, (PyObject *)key);
      double thisval = 0.0;
      if(item != NULL)
        thisval = PyFloat_AS_DOUBLE((PyFloatObject *)item);
      double newvalue = thisval + otherval * c_value;
      if(newvalue == 0.0) {
        PyDict_DelItem((PyObject *)self, key);
      } else {
        // replace current float with a new float with the new value
        // TODO: not sure how to handle deletion and reference counting here!!
        PyObject *newfloat = PyFloat_FromDouble(newvalue);
        //Py_INCREF(newfloat); // ???
        PyDict_SetItem((PyObject *)self, key, newfloat);
      }
    }
    Py_RETURN_NONE; // "void" type function
}




PyDoc_STRVAR(sparsevec_copy_doc, "D.copy() -> a shallow copy of D.");

static PyObject *
sparsevec_copy(sparsevecobject *dd)
{
    /* This calls the object's class.  That only works for subclasses
       whose class constructor has the same signature.  Subclasses that
       define a different constructor signature must override copy().
    */
    // TODO: should we check here that we copy only compatible default dicts?
    return PyObject_CallFunctionObjArgs((PyObject*)Py_TYPE(dd), dd, NULL);
}

static PyObject *
sparsevec_reduce(sparsevecobject *dd)
{
    /* __reduce__ must return a 5-tuple as follows:

       - factory function
       - tuple of args for the factory function
       - additional state (here None)
       - sequence iterator (here None)
       - dictionary iterator (yielding successive (key, value) pairs

       This API is used by pickle.py and copy.py.

       Both shallow and deep copying are supported, but for deep
       copying, the default_factory must be deep-copyable; e.g. None,
       or a built-in (functions are not copyable at this time).

       This only works for subclasses as long as their constructor
       signature is compatible; the first argument must be the
       optional default_factory, defaulting to None.
    */
    PyObject *args;
    PyObject *items;
    PyObject *iter;
    PyObject *result;
    _Py_IDENTIFIER(items);

    args = PyTuple_New(0);
    if (args == NULL)
        return NULL;
    items = _PyObject_CallMethodId((PyObject *)dd, &PyId_items, NULL);
    if (items == NULL) {
        Py_DECREF(args);
        return NULL;
    }
    iter = PyObject_GetIter(items);
    if (iter == NULL) {
        Py_DECREF(items);
        Py_DECREF(args);
        return NULL;
    }
    result = PyTuple_Pack(5, Py_TYPE(dd), args,
                          Py_None, Py_None, iter);
    Py_DECREF(iter);
    Py_DECREF(items);
    Py_DECREF(args);
    return result;
}

// TODO: overwrite set and prevent any element type that is not convertible to float?
// convert everything to float. ...
static PyMethodDef sparsevec_methods[] = {
    {"__missing__", (PyCFunction)sparsevec_missing, METH_O,
     sparsevec_missing_doc},
    {"copy", (PyCFunction)sparsevec_copy, METH_NOARGS,
     sparsevec_copy_doc},
    {"__copy__", (PyCFunction)sparsevec_copy, METH_NOARGS,
     sparsevec_copy_doc},
    {"dot", (PyCFunction)sparsevec_dot, METH_O, "fast dot product"},
    {"iaddc", (PyCFunction)sparsevec_iaddc, METH_VARARGS, "increment all matching key values by weighted other values"},
    {"__reduce__", (PyCFunction)sparsevec_reduce, METH_NOARGS,
     "reduce doc string"},
    {NULL}
};

static PyMemberDef sparsevec_members[] = {
    {NULL}
};

static void
sparsevec_dealloc(sparsevecobject *dd)
{
    PyDict_Type.tp_dealloc((PyObject *)dd);
}

static PyObject *
sparsevec_repr(sparsevecobject *dd)
{
    PyObject *baserepr, *result;
    baserepr = PyDict_Type.tp_repr((PyObject *)dd);
    if (baserepr == NULL)
        return NULL;
    result = PyUnicode_FromFormat("SparseVector(%U)",baserepr);
    Py_DECREF(baserepr);
    return result;
}

static int
sparsevec_traverse(PyObject *self, visitproc visit, void *arg)
{
    return PyDict_Type.tp_traverse(self, visit, arg);
}

static int
sparsevec_tp_clear(sparsevecobject *dd)
{
    return PyDict_Type.tp_clear((PyObject *)dd);
}

static int
sparsevec_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *newargs;
    if (args == NULL || !PyTuple_Check(args))
        newargs = PyTuple_New(0);
    else {
        Py_ssize_t n = PyTuple_GET_SIZE(args);
        newargs = PySequence_GetSlice(args, 0, n);
    }
    int result = PyDict_Type.tp_init(self, newargs, kwds);
    Py_DECREF(newargs);
    return result;
}

PyDoc_STRVAR(sparsevec_doc,
"SparseVector([...]) --> defaultdict float factory and additional methods\n\
\n\
This returns 0.0 for missing items and provides the following additional\n\
methods: dot, iaddc\n\
");

/* See comment in xxsubtype.c */
#define DEFERRED_ADDRESS(ADDR) 0

static PyTypeObject sparsevec_type = {
    PyVarObject_HEAD_INIT(DEFERRED_ADDRESS(&PyType_Type), 0)
    "sparsevectors.SparseVector",          /* tp_name */
    sizeof(sparsevecobject),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)sparsevec_dealloc,        /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    (reprfunc)sparsevec_repr,             /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
                                    /* tp_flags */
    sparsevec_doc,                        /* tp_doc */
    sparsevec_traverse,                   /* tp_traverse */
    (inquiry)sparsevec_tp_clear,          /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset*/
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    sparsevec_methods,                    /* tp_methods */
    sparsevec_members,                    /* tp_members */
    0,                                  /* tp_getset */
    DEFERRED_ADDRESS(&PyDict_Type),     /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    sparsevec_init,                       /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
    0,                                  /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

/* helper function for Counter  *********************************************/

PyDoc_STRVAR(_count_elements_doc,
"_count_elements(mapping, iterable) -> None\n\
\n\
Count elements in the iterable, updating the mapping");

static PyObject *
_count_elements(PyObject *self, PyObject *args)
{
    _Py_IDENTIFIER(get);
    _Py_IDENTIFIER(__setitem__);
    PyObject *it, *iterable, *mapping, *oldval;
    PyObject *newval = NULL;
    PyObject *key = NULL;
    PyObject *zero = NULL;
    PyObject *one = NULL;
    PyObject *bound_get = NULL;
    PyObject *mapping_get;
    PyObject *dict_get;
    PyObject *mapping_setitem;
    PyObject *dict_setitem;

    if (!PyArg_UnpackTuple(args, "_count_elements", 2, 2, &mapping, &iterable))
        return NULL;

    it = PyObject_GetIter(iterable);
    if (it == NULL)
        return NULL;

    one = PyLong_FromLong(1);
    if (one == NULL)
        goto done;

    /* Only take the fast path when get() and __setitem__()
     * have not been overridden.
     */
    mapping_get = _PyType_LookupId(Py_TYPE(mapping), &PyId_get);
    dict_get = _PyType_LookupId(&PyDict_Type, &PyId_get);
    mapping_setitem = _PyType_LookupId(Py_TYPE(mapping), &PyId___setitem__);
    dict_setitem = _PyType_LookupId(&PyDict_Type, &PyId___setitem__);

    if (mapping_get != NULL && mapping_get == dict_get &&
        mapping_setitem != NULL && mapping_setitem == dict_setitem) {
        while (1) {
            /* Fast path advantages:
                   1. Eliminate double hashing
                      (by re-using the same hash for both the get and set)
                   2. Avoid argument overhead of PyObject_CallFunctionObjArgs
                      (argument tuple creation and parsing)
                   3. Avoid indirection through a bound method object
                      (creates another argument tuple)
                   4. Avoid initial increment from zero
                      (reuse an existing one-object instead)
            */
            Py_hash_t hash;

            key = PyIter_Next(it);
            if (key == NULL)
                break;

            if (!PyUnicode_CheckExact(key) ||
                (hash = ((PyASCIIObject *) key)->hash) == -1)
            {
                hash = PyObject_Hash(key);
                if (hash == -1)
                    goto done;
            }

            oldval = _PyDict_GetItem_KnownHash(mapping, key, hash);
            if (oldval == NULL) {
                if (PyErr_Occurred())
                    goto done;
                if (_PyDict_SetItem_KnownHash(mapping, key, one, hash) < 0)
                    goto done;
            } else {
                newval = PyNumber_Add(oldval, one);
                if (newval == NULL)
                    goto done;
                if (_PyDict_SetItem_KnownHash(mapping, key, newval, hash) < 0)
                    goto done;
                Py_CLEAR(newval);
            }
            Py_DECREF(key);
        }
    } else {
        bound_get = _PyObject_GetAttrId(mapping, &PyId_get);
        if (bound_get == NULL)
            goto done;

        zero = PyLong_FromLong(0);
        if (zero == NULL)
            goto done;

        while (1) {
            key = PyIter_Next(it);
            if (key == NULL)
                break;
            oldval = PyObject_CallFunctionObjArgs(bound_get, key, zero, NULL);
            if (oldval == NULL)
                break;
            newval = PyNumber_Add(oldval, one);
            Py_DECREF(oldval);
            if (newval == NULL)
                break;
            if (PyObject_SetItem(mapping, key, newval) < 0)
                break;
            Py_CLEAR(newval);
            Py_DECREF(key);
        }
    }

done:
    Py_DECREF(it);
    Py_XDECREF(key);
    Py_XDECREF(newval);
    Py_XDECREF(bound_get);
    Py_XDECREF(zero);
    Py_XDECREF(one);
    if (PyErr_Occurred())
        return NULL;
    Py_RETURN_NONE;
}

/* module level code ********************************************************/

PyDoc_STRVAR(module_doc,
"High performance data structures.\n\
- SparseVector:  a defaultdirct with default value type and additional methods\n\
");

static struct PyMethodDef module_functions[] = {
    {"_count_elements", _count_elements,    METH_VARARGS,   _count_elements_doc},
    {NULL,       NULL}          /* sentinel */
};

static struct PyModuleDef _sparsevectorsmodule = {
    PyModuleDef_HEAD_INIT,
    "_sparsevectors",
    module_doc,
    -1,
    module_functions,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit_sparsevectors(void)
{
    PyObject *m;

    m = PyModule_Create(&_sparsevectorsmodule);
    if (m == NULL)
        return NULL;

    sparsevec_type.tp_base = &PyDict_Type;
    if (PyType_Ready(&sparsevec_type) < 0)
        return NULL;
    Py_INCREF(&sparsevec_type);
    PyModule_AddObject(m, "SparseVector", (PyObject *)&sparsevec_type);

    return m;
}
