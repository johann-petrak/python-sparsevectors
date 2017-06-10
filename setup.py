import os
from setuptools import setup
# from distutils.core import setup
from distutils.extension import Extension

here = os.path.abspath(os.path.dirname(__file__))
def read(fname):
      return open(os.path.join(here, fname)).read()
readme = read('README.rst')


setup(name = "sparsevectors",
      version = "1.0",
      description="Simple Sparse vector type based on defaultdict(float).",
      long_description=readme,
      author="Johann Petrak",
      author_email="johann.petrak@gmail.com",
      url="https://todo.org/todo",
      ext_modules = [Extension("sparsevectors", ["src/_sparsevectorsmodule.c"])],
      tests_require = ['nose'],
      test_suite = 'nose.collector',
      classifiers=[
          'Development Status LL 4 - Beta',
          'License :: OSI Approved :: Apache 2 License',
          'Programming Language :: Python',
          'Programming Language :: C',
          ],
      )
