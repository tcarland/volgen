NEED_SOCKET = 1
NEED_PTHREADS = 1
NEED_TCANETPP = 1

ifdef TCAMAKE_DEBUG
OPT_FLAGS = -g
endif

CXXFLAGS=   -std=c++23
LIBS =      -lrt
INCLUDES =  -Iinclude

BIN =  	    volgen
OBJS =      src/VolGen.o src/volgen_main.o

ALL_OBJS =  $(OBJS)
ALL_BINS =  $(BIN)

# -------------------------

ifeq ($(TCAMAKE_HOME),)
	TCAMAKE_HOME := $(shell realpath ../tcamake)
endif

include $(TCAMAKE_HOME)/tcamake_include

# -------------------------

all: volgen

volgen: $(OBJS)
	$(make-cxxbin-rule)

clean:
	$(RM) $(OBJS) \
	*.d *.D *.bd src/*d src/*.D src/*.bd

distclean: clean
	$(RM) $(BIN)

install:
ifdef TCAMAKE_PREFIX
	( cp volgen $(TCAMAKE_PREFIX)/bin/ )
	( cp bin/mkiso.sh $(TCAMAKE_PREFIX)/bin )
	( cp bin/voldiff.sh $(TCAMAKE_PREFIX)/bin )
endif
