#!/bin/bash

ROOT=`pwd`

rm -rf build
./autogen.sh
if [ $? -ne 0 ]; then
    echo "autogen failed"
    exit 1
fi
mkdir build
cd build
../configure --prefix=/tmp/estar-test
if [ $? -ne 0 ]; then
    echo "configure failed"
    cd $ROOT
    exit 1
fi
make depend
if [ $? -ne 0 ]; then
    echo "make depend failed"
    cd $ROOT
    exit 1
fi
make
if [ $? -ne 0 ]; then
    echo "make failed"
    cd $ROOT
    exit 1
fi
echo "PIECE OF CAKE!"
cd $ROOT
