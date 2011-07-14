OclUtils
================================

Content
-------------------------

C++ library to initialize a list of OpenCL platforms and the available (sorted) list of devices on each platforms.

Compilation
-------------------------

To compile, you'll need [CMake version 2.4](http://www.cmake.org/) or up.
``$ git clone git://github.com/nbigaouette/oclutils.git
$ cd oclutils
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_INSTALL_PREFIX="/usr"
$ make``
Then to install:
``$ sudo make install``
This will install the include file into /usr/include and the
libraries into /usr/lib



License
-------------------------

This code is distributed under the terms of the [GNU General Public License v3 (GPLv3)](http://www.gnu.org/licenses/gpl.html) and is Copyright 2011 Nicolas Bigaouette
