#Project

This is a template for a C++ project. It's a good starting point. Clone and start hacking.

    git clone https://github.com/evpo/cpp_project_template.git cpp_project
    cd cpp_project/deps
    wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz
    tar xf release-1.8.0.tar.gz
    move googletest-release-1.8.0 gtest
    rm release-1.8.0.tar.gz

CLI:

    ./bin/debug/cpp-project

Unit tests:

    ./bin/debug/cpp-project-tests

##System requirements

    Linux
    Mingw
    C++11 g++ or clang

##Compile

Build:

    make

Release:

    make RELEASE=on

Verbose:

    make VERBOSE=on

Clean:

    make clean

##Acknowledgments

1. [stlplus](http://stlplus.sourceforge.net/)
2. [gtest](https://github.com/google/googletest)

##License

Creative Commons BSD License
