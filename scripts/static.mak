.PHONY: all clean
include ../deps/makefiles/platform.mak
include all_obj.mak
CPP_OBJECTS := $(ALL_OBJ)
OBJECTS := $(CPP_OBJECTS)

ifeq ($(RELEASE),on)
	CONF=release
else
	CONF=debug
endif

LIBRARY := ../bin/$(CONF)/libencryptmsg.a

$(LIBRARY): $(OBJECTS)
	@echo "$(LIBNAME):$(SUBDIR): Updating library $@"
	@$(AR) crs $(LIBRARY) $(OBJECTS)

all : $(LIBRARY)

clean:
	rm -f $(LIBRARY)


