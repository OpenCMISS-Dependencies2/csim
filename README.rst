This is a reboot of the CSim project which aims to better support the use of CellML models in SED-ML tools and OpenCMISS.
Please consider this very much a **work in progress**.

The initial aim is to separate the generation of an executable model from the actual numerical method being used to integrate
the model, and borrows heavily from code written for OpenCMISS (https://github.com/opencmiss/cellml) - which is now being
migrated to use CSim instead.

CSim
====

A library to help application developers execute CellML models. See http://get.readthedocs.org/en/latest/csim/index.html

Building from Source
====================

The following is a list of requirements required for building on all platforms

 #. Toolchain (Visual Studio, GNU/GCC, Clang)
 #. Git distributed version control system
 #. CMake version 3.3 or greater
 #. Python, including development header files

CSim uses a super build system to build all requirements and CSim itself.  This is a two stage process, first the requirements are configured and built and second CSim is configured and built.

For all systems first acquire the CSim source code::

  git clone --recursive here

then create a sibling directory to the source where all the building of CSim will take place::

  mkdir build
  cd build

See the platform specific section for the configuration and build phase of CSim.  The default build of CSim is a release build use the CMAKE_BUILD_TYPE variable to configure the build type for CSim's requirements and CSIM_BUILD_TYPE to configure the build type for CSim itself.  This of course is only relevant to single configuration builds like ninja and make.

Windows
-------

Ninja Generator
+++++++++++++++

When choosing to use the Ninja generator on Windows make sure you set the environment correctly using the vcvarsall.bat script, for example to set a 64 bit build for Visual Studio 2015 in a cmd window::

"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

Configure the requirements build (or phase 1)::

  cmake -G Ninja ..\csim

start the requirements build::

  cmake --build .

(Don't forget the fullstop at the end of the command above it is important.)

Next configure CSim::

  cmake -G Ninja -DUSE_SUPERBUILD=FALSE -DCSIM_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<absoulte-path-to-build-directory>/local/cmake -DBUILD_TESTING=TRUE ..\csim

start the build::

  cmake --build .

To check the build run the tests with the following command::

  ctest

Visual Studio
+++++++++++++

Follow these instructions for building 64 bit CSim from the command line using Visual Studio.

Configure the requirements build::

  cmake -G"Visual Studio 14 2015 Win64" ..\csim

start the requirements build::

  cmake --build . --config Release

Next configure CSim::

  cmake -G"Visual Studio 14 2015 Win64" -DUSE_SUPERBUILD=FALSE -DCSIM_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<absoulte-path-to-build-directory>/local/cmake -DBUILD_TESTING=TRUE ..\csim

start the build::

  cmake --build .  --config Release

To check the build run the tests with the following command::

  ctest

It is also possible to build CSim from the Visual Studio solution in the build directory.