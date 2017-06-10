#!/usr/bin/env python

from sparsevectors import SparseVector

print("create d")
d = SparseVector()  

print("d=",d)
print("set el 1 to 2.2")
d[1] = 2.2
print("done")
print("d[1]=",d[1])
print("Nonexistent: d[100]=",d[100])
print("d=",d,"d[1]=",d[1],"d[2]=",d[2],"d[100]=",d[100])
print("len(d)=",len(d))

dcopy = SparseVector(d)
print("Copy of d, dcopy=",dcopy)
print("set d[1] to 5.5")
d[1]=5.5
#d[2]="asas"
print("d=",d,"dcopy=",dcopy)

print("create e")
e = SparseVector()
print("set el 1 to 2.0")
e[1] = 2.0
#e[3] = "aaaa"
print("e=",e)
print("calculating dot")
r1 = d.dot(e)
print("dot=",r1)

print("incrementing d by 2 * e")
d.iaddc(e,2.0)

print("d=",d,"dcopy=",dcopy,"e=",e)

#print("Testing type errors")
#s1 = SparseVector({1: "sasas"})
#d.iaddc(s1,2.0)
