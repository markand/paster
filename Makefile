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

CC=             cc
CFLAGS=         -std=c18 -pedantic -D_XOPEN_SOURCE=700 -g
# Release
# CFLAGS=         -std=c18 -Wall -Wextra -pedantic -O3 -DNDEBUG -D_XOPEN_SOURCE=700
LDFLAGS=        -lkcgi -lz

SRCS=           config.c database.c http.c log.c pasterd.c paste.c util.c
OBJS=           ${SRCS:.c=.o}
DEPS=           ${SRCS:.c=.d}

SQLITE_FLAGS=   -DSQLITE_THREADSAFE=0 \
                -DSQLITE_OMIT_LOAD_EXTENSION \
                -DSQLITE_OMIT_DEPRECATED \
                -DSQLITE_DEFAULT_FOREIGN_KEYS=1

PREFIX=         /usr/local
BINDIR=         ${PREFIX}/bin
SHAREDIR=       ${PREFIX}/share
VARDIR=         ${PREFIX}/var

DEFINES=        -DSHAREDIR=\"${SHAREDIR}\" -DVARDIR=\"${VARDIR}\"

.SUFFIXES:
.SUFFIXES: .c .o

all: pasterd paster

-include ${DEPS}

.c.o:
	${CC} ${CFLAGS} ${DEFINES} -MMD -Iextern -c $<

extern/sqlite3.o: extern/sqlite3.c extern/sqlite3.h
	${CC} ${CFLAGS} ${SQLITE_FLAGS} -MMD -c $< -o $@

extern/libsqlite3.a: extern/sqlite3.o
	${AR} -rc $@ $<

pasterd: ${OBJS} extern/libsqlite3.a
	${CC} -o $@ ${OBJS} ${LDFLAGS} extern/libsqlite3.a

paster: paster.sh
	cp paster.sh paster
	chmod +x paster

clean:
	rm -f extern/sqlite3.o extern/libsqlite3.a
	rm -f pasterd paster ${OBJS} ${DEPS}

install:
	mkdir -p ${DESTDIR}${BINDIR}
	cp pasterd ${DESTDIR}${BINDIR}
	cp paster ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${SHAREDIR}/paster
	cp -R themes ${DESTDIR}${SHAREDIR}/paster

.PHONY: all clean run
