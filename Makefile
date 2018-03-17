.PHONY: src tests cli gtest symlinks clean-symlinks

include deps/makefiles/platform.mak
include build/config.mak

CONFIG_DIR :=
ifeq ($(RELEASE),on)
	CONFIG_DIR := release
else
	CONFIG_DIR := debug
endif

all: shared
gtest:
	$(MAKE) -C tests -f Makefile.gtest RELEASE=$(RELEASE)

tests: gtest | cli
	$(MAKE) -C tests RELEASE=$(RELEASE)
	$(MAKE) -C test_assets RELEASE=$(RELEASE)

cli : src
	$(MAKE) -C cli RELEASE=$(RELEASE)

lib: src

shared: symlinks
	$(MAKE) -C src BUILD_SHARED=on

src : symlinks
	$(MAKE) -C src RELEASE=$(RELEASE) BOTAN_CXXFLAGS=$(BOTAN_CXXFLAGS) OVERRIDE_BOTAN_FLAGS=$(OVERRIDE_BOTAN_FLAGS)
	$(MAKE) -C deps/state_machine RELEASE=$(RELEASE) TRACE_STATE_MACHINE=$(TRACE_STATE_MACHINE)

install: shared
	cp -f ./bin/$(CONFIG_DIR)/libencryptmsg.so $(PREFIX)/$(LIBDIR)/libencryptmsg.so.$(VERSION)
	ln -f -s libencryptmsg.so.$(VERSION) $(PREFIX)/$(LIBDIR)/libencryptmsg.so
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/libencryptmsg
	cp -fr ./include/encryptmsg $(PREFIX)/$(INCLUDEDIR)/libencryptmsg/
	mkdir -p $(PREFIX)/$(LIBDIR)/pkgconfig
	cp -f build/libencryptmsg.pc $(PREFIX)/$(LIBDIR)/pkgconfig

clean:
	$(MAKE) -C tests -f Makefile.gtest clean RELEASE=$(RELEASE)
	$(MAKE) -C tests clean RELEASE=$(RELEASE)
	$(MAKE) -C cli clean RELEASE=$(RELEASE)
	$(MAKE) -C src clean RELEASE=$(RELEASE)
	$(MAKE) -C deps/state_machine clean RELEASE=$(RELEASE)
	$(MAKE) -C test_assets clean RELEASE=$(RELEASE)
