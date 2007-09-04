#!/bin/bash

PREFIX="$PWD/stage"
EXTRA_CFGOPTS=""
RUN_BOOTSTRAP="yes"
WORKDIR="build"
EXTRA_MAKEOPTS=""
MAKE="make"

while [ ! -z "$1" ]; do
    case $1 in
	-h|--help)
echo "build-stage.sh command line options:"
echo "  [-p|--prefix]  <PREFIX>     install prefix ($PREFIX)"
echo "  [-w|--work]    <DIR>        build work directory ($WORKDIR)"
echo "  [-b|--boost]   <DIR>        BOOST library install directory"
echo "  [-j|--jobs]    <NUM>        number of parallel make jobs"
echo "  [-m|--make]    <PATH>       GNU Make executable (name or path)"
echo "  [-d|--debug]                enable debug messages and symbols"
echo "  [-s|--skipbs]               do not bootstrap build system"
        exit 0;;
	-p|--prefix)
	    PREFIX=$2;
	    shift; shift; continue;;
	-w|--work)
	    WORKDIR=$2;
	    shift; shift; continue;;
	-b|--boost)
	    EXTRA_CFGOPTS="$EXTRA_CFGOPTS --with-boost=$2"
	    shift; shift; continue;;
	-j|--jobs)
	    EXTRA_MAKEOPTS="$EXTRA_MAKEOPTS -j $2"
	    shift; shift; continue;;
	-m|--make)
	    MAKE="$2"
	    shift; shift; continue;;
	-d|--debug)
	    EXTRA_CFGOPTS="$EXTRA_CFGOPTS --enable-debug"
	    shift; continue;;
	-s|--skipbs)
	    RUN_BOOTSTRAP="no"
	    shift; continue;;
	*)
	    echo "ERROR unhandled option(s) $*" 1>&2
	    exit 1;;
    esac
done

CFGOPTS="--prefix=$PREFIX $EXTRA_CFGOPTS"

echo "configure options: $CFGOPTS"

if [ $RUN_BOOTSTRAP = "yes" ]; then
    echo "running bootstrap-buildsystem.sh (takes a while)"
    ./bootstrap-buildsystem.sh
    if [ $? -ne 0 ]; then
	echo "ERROR bootstrap-buildsystem.sh"
	exit 1
    fi
fi

rm -rf $WORKDIR
if [ ! -d $WORKDIR ]; then
    mkdir $WORKDIR
    if [ $? -ne 0 ]; then
	echo "ERROR mkdir $WORKDIR"
	exit 1
    fi
fi

cd $WORKDIR
../configure $CFGOPTS
if [ $? -ne 0 ]; then
    echo "ERROR ../configure $CFGOPTS"
    exit 1
fi

$MAKE $EXTRA_MAKEOPTS
if [ $? -ne 0 ]; then
    echo "ERROR $MAKE $EXTRA_MAKEOPTS"
    exit 1
fi

$MAKE $EXTRA_MAKEOPTS install
if [ $? -ne 0 ]; then
    echo "ERROR $MAKE $EXTRA_MAKEOPTS install"
    exit 1
fi
