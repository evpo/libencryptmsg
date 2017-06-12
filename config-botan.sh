#/bin/bash
pushd ./deps/botan2
./configure.py --enable-modules=aes,pbkdf2,zlib --link-method=symlink
popd
