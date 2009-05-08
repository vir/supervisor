# my makefile

BINS = supervisor

all: .depend ${BINS}

.SUFFIXES: .cxx .so

.c.o:
	gcc -g -Wall -pipe -c -o $*.o $<

.cxx.o:
	g++ -g -Wall -pipe -c -o $*.o $<
        
.o.so:
	gcc -g -Wall -pipe -shared -nostartfiles -nostdlib -o $@ $^

supervisor: supervisor.o family.o childprocess.o eventdispatcher.o config.o logger.o
	g++ -g -Wall -pipe -o $@ $^

#---------------------------------------------------------------------------
.PHONY: clean
clean:
	-rm -f ${BINS} *.o .depend core

#install:
#	install -d ${DIR_BINS}
#	install -d ${DIR_LIBS}
#	install -s -m 500 ${BINS} ${DIR_BINS}
#	install    -m 500 ${DLLS} ${DIR_LIBS}

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif




