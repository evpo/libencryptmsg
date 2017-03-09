include ../deps/makefiles/platform.mak

LIBRARIES += ../src ../deps/stlplus/subsystems ../deps/stlplus/containers ../deps/stlplus/portability

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

CXXFLAGS += -Wall -Wextra -std=c++11
CXXFLAGS += -I../include
LDFLAGS += -Wall
LOADLIBES += -lm

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/cpp-project
include ../deps/makefiles/gcc.mak
