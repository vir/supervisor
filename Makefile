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

supervisor: supervisor.o family.o childprocess.o eventdispatcher.o config.o logger.o daemonize.o pidfile.o
	g++ -g -Wall -pipe -o $@ $^

#---------------------------------------------------------------------------
.PHONY: clean
clean:
	-rm -f ${BINS} *.o .depend core

install: supervisor
	install -m 555 supervisor /usr/local/bin

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif




