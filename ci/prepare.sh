#!/bin/bash
set -exu

env_name=$1

git clone https://github.com/randombit/botan $HOME/botan
( cd $HOME/botan && git checkout 2.7.0 )
pushd $HOME/botan
    if [[ "$env_name" == "appveyor" ]]; then
        ./configure.py --prefix=./out --enable-modules=aes,pbkdf2,auto_rng,compression,zlib --amalgamation --disable-shared --cc gcc --cpu x86 --os mingw
    elif [[ "$env_name" == "osx" ]]; then
        ./configure.py --prefix=./out --enable-modules=aes,pbkdf2,auto_rng,compression,zlib --link-method=symlink --amalgamation --disable-shared
    elif [[ "$env_name" == "linux" ]]; then
        ./configure.py --prefix=/usr --enable-modules=aes,pbkdf2,zlib --link-method=symlink
    fi
popd
