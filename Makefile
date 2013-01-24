TOPDIR = ../../tnms

NEED_SOCKET = 1
NEED_PTHREADS = 1
NEED_TCANETPP = 1

ifdef TNMS_DEBUG
OPT_FLAGS =     -g
endif

LIBS =  	-lrt
INCLUDES =      -I.

BIN =		volgen
OBJS =		VolGen.o volgen_main.o

ALL_OBJS =      $(OBJS)
ALL_BINS =      $(BIN)


all: volgen


include $(TOPDIR)/tcamake/project_defs


volgen: $(OBJS)
	$(make-cxxbin-rule)


clean:
	$(RM) $(OBJS) \
	*.d *.D *.bd 

distclean: clean
	$(RM) $(BIN)

install:
	
