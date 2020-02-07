#
# Makefile -- basic makefile for paster
#
# Copyright (c) 2020 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

.POSIX:

# User options.
CC=             cc
CFLAGS=         -DNDEBUG -O3

# Installation paths.
PREFIX=         /usr/local
BINDIR=         ${PREFIX}/bin
SHAREDIR=       ${PREFIX}/share
MANDIR=         ${PREFIX}/share/man
VARDIR=         ${PREFIX}/var

VERSION=        0.1.0

CORE_SRCS=      config.c database.c http.c log.c paste.c util.c
CORE_HDRS=      config.h database.h http.h log.h paste.h util.h
CORE_OBJS=      ${CORE_SRCS:.c=.o}
CORE_DEPS=      ${CORE_SRCS:.c=.d}

SQLITE_FLAGS=   -DSQLITE_THREADSAFE=0 \
                -DSQLITE_OMIT_LOAD_EXTENSION \
                -DSQLITE_OMIT_DEPRECATED \
                -DSQLITE_DEFAULT_FOREIGN_KEYS=1

MY_CFLAGS=      -std=c18 \
                -Iextern \
                -D_XOPEN_SOURCE=700 \
                -DSHAREDIR=\"${SHAREDIR}\" \
                -DVARDIR=\"${VARDIR}\" \
                ${CFLAGS}
MY_LDFLAGS=     -static -lkcgi -lkcgihtml -lz ${LDFLAGS}

.SUFFIXES:
.SUFFIXES: .c .o .in

all: pasterd pasterd-clean paster

-include ${CORE_DEPS}
-include pasterd.d
-include pasterd-clean.d

.c.o:
	${CC} ${MY_CFLAGS} -MMD -Iextern -c $<

.in:
	sed -e "s|@SHAREDIR@|${SHAREDIR}|" \
	    -e "s|@VARDIR@|${VARDIR}|" \
	    < $< > $@

extern/sqlite3.o: extern/sqlite3.c extern/sqlite3.h
	${CC} ${MY_CFLAGS} ${SQLITE_FLAGS} -c $< -o $@

extern/libsqlite3.a: extern/sqlite3.o
	${AR} -rc $@ extern/sqlite3.o

pasterd: ${CORE_OBJS} extern/libsqlite3.a pasterd.o pasterd.8
	${CC} -o $@ ${CORE_OBJS} pasterd.o ${MY_LDFLAGS} extern/libsqlite3.a

pasterd-clean: ${OBJS} extern/libsqlite3.a pasterd-clean.o pasterd-clean.8
	${CC} -o $@ ${CORE_OBJS} pasterd-clean.o ${MY_LDFLAGS} extern/libsqlite3.a

paster: paster.sh paster.1
	cp paster.sh paster
	chmod +x paster

clean:
	rm -f extern/sqlite3.o extern/libsqlite3.a
	rm -f ${CORE_OBJS} ${CORE_DEPS}
	rm -f pasterd pasterd.d pasterd.o pasterd.8
	rm -f pasterd-clean pasterd-clean.d pasterd-clean.o pasterd-clean.8
	rm -f paster paster.1

install-paster:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	cp paster ${DESTDIR}${BINDIR}
	cp paster.1 ${DESTDIR}${MANDIR}/man1/paster.1

install-pasterd:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man8
	cp pasterd ${DESTDIR}${BINDIR}
	cp pasterd-clean ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${SHAREDIR}/paster
	cp -R themes ${DESTDIR}${SHAREDIR}/paster
	cp pasterd.8 ${DESTDIR}${MANDIR}/man8/pasterd.8

install: install-pasterd install-paster

dist: clean
	mkdir -p paster-${VERSION}
	cp -R extern paster-${VERSION}
	cp -R themes paster-${VERSION}
	cp ${CORE_SRCS} ${CORE_HDRS} paster-${VERSION}
	cp pasterd.8.in pasterd.c paster-${VERSION}
	cp pasterd-clean.8.in pasterd-clean.c paster-${VERSION}
	cp paster.1.in paster.sh paster-${VERSION}
	cp Makefile CHANGES.md CONTRIBUTE.md CREDITS.md INSTALL.md LICENSE.md \
	    README.md STYLE.md TODO.md paster-${VERSION}
	tar -cJf paster-${VERSION}.tar.xz paster-${VERSION}
	rm -rf paster-${VERSION}

.PHONY: all clean dist run
