include ../deps/makefiles/platform.mak
# Uncomment to add libraries
LIBRARIES += ../deps/state_machine ../deps/stlplus/containers ../deps/botan_1_openpgp_codec
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra

CXXFLAGS += -I./ -I../include/encryptmsg -I../deps/plog/include $(BOTAN_CXXFLAGS)
ifneq ($(OVERRIDE_BOTAN_FLAGS),on)
CXXFLAGS += $(shell pkg-config --cflags botan-2)
endif

LDFLAGS += -Wall

ifeq ($(BUILD_SHARED),on)
LDLIBS += -lm -lbotan-2
ifeq ($(RELEASE),on)
TARGETDIR := ../bin/release
else
TARGETDIR := ../bin/debug
endif

ifeq ($(WINDOWS),on)
SHARED_SO := $(TARGETDIR)/encryptmsg.dll
else
SHARED_SO := $(TARGETDIR)/libencryptmsg.so
endif

endif

include ../deps/makefiles/gcc.mak
