#!/bin/sh

cd `dirname $0`

# delete build directory
rm -rf build
rm -f multiverse
rm -f multiverse-cli
rm -f multiverse-server
rm -f multiverse-miner
rm -f multiverse-dnseed

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
make -j8
if [ $? -ne 0 ]; then 
    exit 1 
fi 

# install
cp src/multiverse ../
cd ..
ln -s multiverse multiverse-cli
ln -s multiverse multiverse-server
ln -s multiverse multiverse-miner
ln -s multiverse multiverse-dnseed
