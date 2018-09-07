#!/bin/sh

cd `dirname $0`

# delete build directory
rm -rf build
rm -f multiverse

# create build directory
mkdir build

# go to build
cd build

# cmake
cmake ..
if [ $? -ne 0 ]; then 
    exit 1 
fi 

# make
make
if [ $? -ne 0 ]; then 
    exit 1 
fi 

# install
cp src/multiverse ../
