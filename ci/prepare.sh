#!/bin/bash
set -exu

git clone https://github.com/randombit/botan $HOME/botan
pushd $HOME/botan
./configure.py --prefix="$HOME/stage/" --with-debug-info --cxxflags="-fno-omit-frame-pointer"
#${MAKE} -j${CORES} install
make install
popd
