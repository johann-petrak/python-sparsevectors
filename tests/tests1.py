from __future__ import print_function
from sparsevectors import SparseVector
import unittest

class SparseVectorTestsBasics(unittest.TestCase):
    def test_sv_basics1(self):
        v1 = SparseVector()
        e1 = v1[0] 
        self.assertEqual(e1, 0.0)
        self.assertNotIn(0, v1)

class SparseVectorTestsDot(unittest.TestCase):

    def test_sv_dot1(self):
        v1 = SparseVector()
        v1[2] = 2.0
        v2 = SparseVector()
        v2[2] = 3.0
        self.assertEqual(type(v1[2]), float)
        d1 = v1.dot(v2)
        self.assertEqual(d1, 6.0)

    def test_sv_dot2(self):
        v1 = SparseVector()
        v2 = SparseVector()
        v2[2] = 3.0
        d1 = v1.dot(v2)
        self.assertEqual(d1, 0.0)


    def test_sv_dot3(self):
        v1 = SparseVector()
        v1[1] = 2.0
        v2 = SparseVector()
        v2[2] = 3.0
        self.assertEqual(type(v1[2]), float)
        d1 = v1.dot(v2)
        self.assertEqual(d1,0.0)

    def test_sv_dot4(self):
        v1 = SparseVector({1: 1.0, 2: 2.0})
        v2 = SparseVector({2: 3.0, 4: 5.0})
        d1 = v1.dot(v2)
        self.assertEqual(d1,6.0)

    def test_sv_dot5(self):
        v1 = SparseVector()
        v2 = SparseVector()
        d1 = v1.dot(v2)
        self.assertEqual(d1,0.0)

class SparseVectorTestsIaddc(unittest.TestCase):

    def test_sv_iaddc1(self):
        v1 = SparseVector({1: 1.0, 2: 2.0})
        v2 = SparseVector({2: 3.0, 4: 5.0})
        v1.iaddc(v2)
        self.assertEqual(len(v1), 3)
        self.assertEqual(len(v2), 2)
        self.assertEqual(v1[1], 1.0)
        self.assertEqual(v1[2], 5.0)
        self.assertEqual(v1[4], 5.0)

    def test_sv_iaddc2(self):
        v1 = SparseVector()
        v2 = SparseVector()
        v2[2] = 2.0
        v1.iaddc(v2)
        self.assertEqual(len(v1), 1)
        self.assertEqual(len(v2), 1)
        self.assertEqual(v1[2], 2.0)

    def test_sv_iaddc3(self):
        v1 = SparseVector()
        v2 = SparseVector()
        v1.iaddc(v2)
        self.assertEqual(len(v1), 0)
        self.assertEqual(len(v2), 0)


if __name__ == "__main__":
    unittest.main()
