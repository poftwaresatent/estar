# E-star Interpolated Graph Replanner

> Copyright (C) 2004-2005 Swiss Federal Institute of Technology, Lausanne
>
> Copyright (C) 2005-2007 Roland Philippsen <roland dot philippsen at gmx net>
>
> Released under the GNU General Public License, version 2. See
> `LICENSE.GPL` for more details.


## Homepage

We still use the [Sourceforge E* project][] for mailing lists and file distribution. The code has moved to a [github repository][] though, and the [github wiki][] may be a good place to look, too.

[Sourceforge E* project]: http://sourceforge.net/projects/estar/
[github repository]: http://github.com/poftwaresatent/estar
[github wiki]: http://github.com/poftwaresatent/estar/wiki


## Description

The E\* algorithm is a path planner for (mobile) robotics. Unlike A\*,
which constrains movements to graph edges, it produces smooth
trajectories by interpolating between edges. Like D\*, it supports
dynamic replanning after local path cost changes. The file `ABOUT` has some more details.


## Dependencies

### Required

* Boost [Graph Library][]
* Boost [Shared Pointers][]

[Graph Library]: http://www.boost.org/libs/graph/
[Shared Pointers]: http://www.boost.org/libs/smart_ptr/

### Optional

* To build the development version, you need GNU [Automake][], [Autoconf][], and [Libtool][]. If you're building from a release tarball, you don't need this.
* An OpenGL implementation. This is sort of optional, the configure script tries to guess if you have OpenGL, but it has never been really tested on a system without it.
* [Doxygen][] for creating API documentation

[Automake]: http://www.gnu.org/software/automake/
[Autoconf]: http://www.gnu.org/software/autoconf/
[Libtool]: http://www.gnu.org/software/libtool/
[Doxygen]: http://www.doxygen.org/


## Building E\*

Release tarballs are available form the [download page][] on Sourceforge.

[download page]: http://sourceforge.net/projects/estar/files/estar/

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

The build-stage script has some more tricks up its sleeve. For example, to install E\* on your system, for example underneath `/usr/local/estar`, just do this:

    $ ./build-stage.sh -p /usr/local/estar


## API Documentation

The [online documentation](http://estar.sourceforge.net/doc/index.html) was generated using [Doxygen][]. Each release can also be downloaded with documentation, simply choose one of the tarballs tagged with-doc on the [download page][]. If you downloaded one of those, then simply open `doc/html/index.html` in a browser.

Otherwise, you can create the API documentation using [Doxygen][] *after* a successful *configuration* from within the `build` directory:

    $ cd build
    $ make doc


## Test

After building and installing, test programs will be in `stage/bin/` (unless you overrode the installation path used by `build-stage.sh`). Some of these test programs are console-based. If you have OpenGL (and the build system detected it), then there will also be graphical programs which are controlled using the keyboard:

* `SPACE`: Compute one propagation step.
* `c`: Continuous propagation, plot each step.
* `f`: Flush - Propagate all cells, then plot.
* `q`: Quit (and optionally write to file)

### Main E\* Test: `test_estar_gfx`

This program is a graphical test of E\*. It expects the name of a grid config file and runs the E\* algorithm using the goal and robot positions (indices) defined therein. You can use the `-t` option to
disable graphical output, in which case it makes sense to also define an output file using `-o filename` in order to capture the result (it simply writes resulting cell values in ASCII).

In the `misc/` directory there are some grid config files that can be used for `test_estar_gfx`:

* `grid-nhp-*.txt` are grid definitions created by Akin Sisbot at LAAS-CNRS for testing E\* on his PhD results for a human-aware motion planner (that was around ca. 2006).
* `grid-small.txt` is a small hand-crafted grid
* `grid-hex-small.txt` is a trial triangular mesh grid... this has eternally been work in progress, triangular meshes are not working reliably.

### Main PNF Test: `test_pnf_gfx`

With this program you can test the PNF algorithm: a planner that takes into account dynamic obstacles and relies on E\* to create smooth navigation functions. More information about PNF can be found in the references.

`test_pnf_gfx` expects the name of a config file. If you specify the string "`paper`" as second argument, then the plots will be in greyscale and with a different layout (supposedly better for inclusion in publications). The `misc/` directory contains some grid config files that can be used for `test_pnf_gfx`:

* `pnf-static-impulse.pnf`: A setup with a single wall between the robot and the goal.
* `pnf-dynamic-impulse.pnf`: A setup with a single dynamic object between the robot and the goal in an otherwise empty environment.
* `pnf-setup-iros06-*.pnf`: Setups for the IROS'06 paper, not all were used though.
* `pnf-setup-star06-*.pnf`: Setups for the STAR paper.

There are also two utility scripts for creating vector-format figures of PNF plots.

* `plot_pnf.sh` uses [Gnuplot][] to create a contour plot of PNF values in [XFig][] format.
* `pnf_3d_riskplot.sh` uses [Gnuplot][] and `fig2dev` to create a 3D plot of PNF risk data in PDF format. (Note that nowadays, you can directly produce PDF output from Gnuplot).

[Gnuplot]: http://www.gnuplot.info/
[XFig]: http://xfig.org/

### Other Programs

* `test_estar_replan_gfx`: Program for comparing two instances of E\*, with the goal of checking that dynamic replanning produces the same results as re-initializing and replanning from scratch. You can use the mouse to add and remove obstacles.
* `test_dbg_opt`: Small test for checking whether debug messages get pruned properly (they do).
* `test_estar`: An old text-only test program for E\*. See the source-code for options.
* `test_estar_queue`: Text-only debugging of the E\* queue-ing mechanism. Will hopefully still be useful one day when work on triangular meshes is taken up again...
* `test_fake_os`: Totally simple compile-test to see whether the rudimentary `estar::fake_os` class behaves correctly.
* `test_pnf_cooc` and `test_pnf_cooc3d`: Old test programs for verifying the co-occurrence probability computations of PNF (which was ported from Matlab code that was written by Bj&ouml;rn Jensen at ASL-EPFL back in 2002 or 2003).
* `test_pnf_riskmap1: Checks that `pnf::PNFRiskMap` does what it should.
* `test_shape`: Tests the `estar::Sprite` class.


## References

* [E* poster](http://estar.sourceforge.net/papers/philippsen-estar-poster-iros07.pdf) presented at IROS 2007 (and the corresponding [flyer](http://estar.sourceforge.net/papers/philippsen-estar-flyer-iros07.pdf)) during the Workshop on Algorithmic Motion Planning for Autonomous Robots in Challenging Environments.
* [E* paper](http://estar.sourceforge.net/papers/philippsen_siegwart_icra2005_paper.pdf) presented at ICRA 2005 (describes an outdated formulation, but gives basic insights)
 ```
    @INPROCEEDINGS{philippsen:2005,
     author    = {Roland Philippsen and Roland Siegwart},
     title     = {An Interpolated Dynamic Navigation Function},
     booktitle = {Proceedings of the IEEE International Conference
                  on Robotics and Automation (ICRA)},
     year      = 2005
    }
  ```
* [Technical Report on "light" E\*](http://estar.sourceforge.net/papers/philippsen--estar-TR06.pdf) written in 2006. This describes the formulation underlying this implementation.
  ```
    @TECHREPORT{philippsen:2006a,
     author      = {Roland Philippsen},
     title       = {A Light Formulation of the E\* Interpolated Path Replanner},
     institution = {Autonomous Systems Lab,
                    Ecole Polytechnique Federale de Lausanne},
     year        = 2006
    }
  ```
* [PNF paper](http://estar.sourceforge.net/papers/philippsen-jensen-siegwart--star06.pdf.zip) published in STAR 2006.
  ```
    @INBOOK{philippsen:2006b,
     author       = {Philippsen, R. and Jensen, B. and Siegwart, R.},
     editor       = {Laugier, C. and Chatila, R.},
     chapter      = {Towards Real-Time Sensor-Based Path Planning
                     in Highly Dynamic Environments},
     title        = {Autonomous Navigation in Dynamic Environments},
     publisher    = {Springer Tracts on Advanced Robotics},
     year         = 2006
    }
  ```
