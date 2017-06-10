========
sparsevectors
========

A library that provides the SparseVector object which is defaultdict(float) with 
the following properties and methods:

- getting a non-existing element always returns 0.0
- method dot(other) returns the dot-product between this SparseVector and another
- method iaddc(other[,weight]) adds to each element in this SparseVector the
  corresponding element of the other SparseVector, multiplied by weight, if given.
  If the result is 0.0, then the element in the current SparseVector is removed


Authors
-------

This code is based on the cpython defaultdict code (see https://github.com/python/cpython)
and is inspired by Liang Huang's (http://web.engr.oregonstate.edu/~huanlian) hvector 
library (http://web.engr.oregonstate.edu/~huanlian/software/hvector-1.0.tar.bz). 
Any errors are mine though.

* Johann Petrak
* Liang Huang 
* Authors and contributors to CPython's _collectionsmodule.c


Status
------

This is still an early version and may contain bugs, memory leaks. 
Any help to make it better or in finding and squashing bugs is welcome.

Installation
------------

TBD


License
-------

Licensed under the terms of the `CPython License`_. See attached file LICENSE.txt.


.. _CPython License: https://github.com/python/cpython/blob/master/LICENSE

