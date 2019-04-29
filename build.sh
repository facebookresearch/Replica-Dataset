#!/usr/bin/env bash

cd 3rdparty/Pangolin
mkdir build
cd build
cmake ..
make -j

cd ../../../
mkdir build
cd build
cmake ..
make -j
