# Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
# License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)

BINS = supervisor
DESTDIR ?=
PREFIX ?= /usr/local

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
	install -m 755 -d ${DESTDIR}${PREFIX}/bin
	install -m 555 supervisor ${DESTDIR}${PREFIX}/bin
	install -m 755 -d ${DESTDIR}${PREFIX}/share/doc
	install -m 644 README.txt LICENSE.txt supervisor.conf ${DESTDIR}${PREFIX}/share/doc

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif




