libpbnjson
==========

Summary
-------
Palm's Better Native JSON library

Description
-----------
A JSON engine, implemented as a pair of libraries with APIs for easier C and
C++ abstraction over the core PBNJSON library.

How to Build on Linux
=====================

## Dependencies

Below are the tools and libraries (and their minimum versions) required to build _libpbnjson_:

* boost-program-options 1.48
* cmake (version required by cmake-modules-webos)
* flex 2.5.7
* gcc 4.6.3
* g++ 4.6.3
* glib 2.34.0
* gmp 5.0.2
* gperf 3.0.3
* lemon 3.7.9
* make (any version)
* webosose/cmake-modules-webos 1.0.0 RC3
* pkg-config 0.26
* uriparser 0.7.5
* yajl 1.0.12 or 2.0.4

Below is the component (and its minimum version) required to use tests:

* gtest 1.7.0

## Building

Once you have downloaded the source, enter the following to build it (after
changing into the directory under which it was downloaded):

    $ mkdir BUILD
    $ cd BUILD
    $ cmake ..
    $ make
    $ sudo make install

The directory under which the files are installed defaults to `/usr/local/webos`.
You can install them elsewhere by supplying a value for `WEBOS_INSTALL_ROOT`
when invoking `cmake`. For example:

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/webosose ..
    $ make
    $ make install

will install the files in subdirectories of `$HOME/projects/webosose`.

Specifying `WEBOS_INSTALL_ROOT` also causes `pkg-config` to look in that tree
first before searching the standard locations. You can specify additional
directories to be searched prior to this one by setting the `PKG_CONFIG_PATH`
environment variable.

If not specified, `WEBOS_INSTALL_ROOT` defaults to `/usr/local/webos`.

To configure for a debug build treating warnings as errors, enter:

    $ cmake -D CMAKE_BUILD_TYPE:STRING=Debug -D WEBOS_USE_WERROR:BOOL=TRUE ..

To see a list of the make targets that `cmake` has generated, enter:

    $ make help

## Uninstalling

From the directory where you originally ran `make install`, enter:

    $ [sudo] make uninstall

You will need to use `sudo` if you did not specify `WEBOS_INSTALL_ROOT`.

## Generating Documentation

The tools required to generate the documentation are:

* doxygen 1.7.6.1
* graphviz 2.26.3

To generate the documentation, add `-D WEBOS_CONFIG_BUILD_DOCS:BOOL=TRUE` to the `cmake`
command line and make the `docs` target:

    $ cmake -D WEBOS_CONFIG_BUILD_DOCS:BOOL=TRUE <other-args> ..
    $ make docs

To view the generated HTML documentation, point your browser to
`Documentation/pbnjson/html/*/index.html`

## Testing

To enable tests add `-D WEBOS_CONFIG_BUILD_TESTS:BOOL=TRUE` to the `cmake`
command line. Add `-D WEBOS_GTEST_SRCDIR:STRING=/path/to/gtest` to specify
path to gtest source, default is `WEBOS_INSTALL_ROOT/src/gtest`.

    $ cmake -DWEBOS_CONFIG_BUILD_TESTS=TRUE \
      -DWEBOS_GTEST_SRCDIR=/path/to/gtest <other-args> ..
    $ make test

# Copyright and License Information

Unless otherwise specified, all content, including all source code files and
documentation files in this repository are:

Copyright (c) 2009-2018 LG Electronics, Inc.

Unless otherwise specified or set forth in the NOTICE file, all content,
including all source code files and documentation files in this repository are:
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

SPDX-License-Identifier: Apache-2.0
