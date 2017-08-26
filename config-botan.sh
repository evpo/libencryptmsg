#/bin/bash
LIB=lib64
PLAT=$(uname -i)
if [[ "$PLAT" == "i386" ]]
then
    LIB=lib
fi

pushd ./deps/botan2
./configure.py --enable-modules=aes,pbkdf2,zlib --link-method=symlink --prefix=/usr --libdir=${LIB}
popd
