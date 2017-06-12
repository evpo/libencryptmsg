include ../deps/makefiles/platform.mak

LIBRARIES += ../src

LDFLAGS += -Wl,--whole-archive
LOADLIBES += -Wl,--no-whole-archive

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

LOADLIBES += ../deps/gtest/googletest/build/libgtest.a -L../deps/botan2 -lbotan-2 -lpthread -lm -Wl,-rpath,../../deps/botan2
CXXFLAGS += -I../deps/gtest/googletest/include -pthread -std=c++11 -Wall -Wextra
CXXFLAGS += -I../include -I../include/internal -I../deps/stlplus/containers -I../deps/botan2/build/include

LDFLAGS += -Wall

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/cpp-project-tests
include ../deps/makefiles/gcc.mak
