# E-star Interpolated Graph Replanner

> Copyright (C) 2004-2005 Swiss Federal Institute of Technology, Lausanne
>
> Copyright (C) 2005-2007 Roland Philippsen <roland dot philippsen at gmx net>
>
> Released under the GNU General Public License, see `LICENSE.GPL`


## Homepage

We still use the [Sourceforge E* project]() for mailing lists and file distribution. The code has moved to a [github repository]() though, and the [github wiki]() may be a good place to look, too.

[Sourceforge E* project]: http://sourceforge.net/projects/estar/
[github repository]: http://github.com/poftwaresatent/estar
[github wiki]: http://github.com/poftwaresatent/estar/wiki


## Description

The E* algorithm is a path planner for (mobile) robotics. Unlike A*,
which constrains movements to graph edges, it produces smooth
trajectories by interpolating between edges. Like D*, it supports
dynamic replanning after local path cost changes. The file `ABOUT` has some more details.


## Dependencies

### Required

* Boost [Graph Library]()
* Boost [Shared Pointers]()

[Graph Library]: http://www.boost.org/libs/graph/
[Shared Pointers]: http://www.boost.org/libs/smart_ptr/

### Optional

* To build the development version, you need GNU [Automake](), [Autoconf](), and [Libtool](). If you're building from a release tarball, you don't need this.
* An OpenGL implementation. This is sort of optional, the configure script tries to guess if you have OpenGL, but it has never been really tested on a system without it.
* [Doxygen]() for creating API documentation

[Automake]: http://www.gnu.org/software/automake/
[Autoconf]: http://www.gnu.org/software/autoconf/
[Libtool]: http://www.gnu.org/software/libtool/
[Doxygen]: http://www.doxygen.org/


## Building E*

Release tarballs are available form the [download]() page on Sourceforge.

[download]: http://sourceforge.net/projects/estar/files/estar/

The instructions here assume you're using the `build-stage.sh` script. It has several options for fine-grained control, read the output of `./build-stage.sh -h` for more information. You can also do a manual "configure / make / make install" of course.

### Download and Build a Release Tarball

    $ tar xfvz estar-X.Y.tar.gz
    $ cd estar-X.Y
    $ ./build-stage.sh -s

Note the `-s` option to the `build-stage.sh` script, which is especially important if you do *not* have GNU Automake, Autoconf, and Libtool. This will build everything using a separate `build` directory, and "install" it in the `stage/` directory.

### Cloning the git Repository

    $ git clone git://github.com/poftwaresatent/estar.git
    $ cd estar
    $ ./build-stage.sh

Note that `build-stage.sh` will create a `build/` directory that can then be used for recompiling the project after modifications to the code. The GNU tools provide dependency tracking, and in most cases only a small portion of the code gets recompiled after a change. Also, you can pass `-s` to subsequent calls to `./build-stage.sh` to avoid wasting time on recreating the build system files that haven't changed.

The build-stage script has some more tricks up its sleeve. For example, to install E* on your system, for example underneath `/usr/local/estar`, just do this:

    $ ./build-stage.sh -p /usr/local/estar


## API Documentation

If you downloaded one of the tarballs that comes with documentation, then simply open `doc/html/index.html` in a browser. Otherwise, you can create the API documentation using [Doxygen](), which works best *after* a successful *configuration* form within the `build` directory:

    $ cd build
    $ make doc
