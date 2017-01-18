TOPDIR = ..

NEED_SOCKET = 1
NEED_PTHREADS = 1
NEED_TCANETPP = 1

ifdef TCAMAKE_DEBUG
OPT_FLAGS =     -g
endif

LIBS =  	-lrt
INCLUDES =      -Iinclude

BIN =		volgen
OBJS =		src/VolGen.o src/volgen_main.o

ALL_OBJS =      $(OBJS)
ALL_BINS =      $(BIN)


all: volgen


include $(TOPDIR)/tcamake/project_defs


volgen: $(OBJS)
	$(make-cxxbin-rule)


clean:
	$(RM) $(OBJS) \
	*.d *.D *.bd src/*d src/*.D src/*.bd

distclean: clean
	$(RM) $(BIN)

install:

