# libEncryptMsg

This is an OpenPGP implementation that works with symmetrically encrypted files following RFC4880 specification.

CLI:

    ./bin/debug/encryptmsg

Unit tests:

    ./bin/debug/encryptmsg-tests

## System requirements

    Linux
    Mingw
    C++11 g++ or clang

## Compile

Build:

    make

Release:

    make RELEASE=on

Verbose:

    make VERBOSE=on

Clean:

    make clean

Tests:

    make tests

## Acknowledgments

1. [**Botan**](http://botan.randombit.net/)
2. [**stlplus**](http://stlplus.sourceforge.net/)
3. [**Makefiles**](http://stlplus.sourceforge.net/makefiles/docs/)
4. [**zlib**](http://zlib.net/)
5. [**gtest**](http://code.google.com/p/googletest/)
6. [**plog**](https://github.com/SergiusTheBest/plog)


## License

libEncryptMsg Copyright 2018 Evgeny Pokhilko 
<https://www.evpo.net/encryptpad>

libencryptmsg is released under the Simplified BSD License (see license.txt)
