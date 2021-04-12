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

VERSION=        0.2.1

CORE_SRCS=      config.c                        \
                database.c                      \
                http.c                          \
                log.c                           \
                fragment-duration.c             \
                fragment-language.c             \
                fragment-paste.c                \
                fragment.c                      \
                page-download.c                 \
                page-fork.c                     \
                page-index.c                    \
                page-new.c                      \
                page-paste.c                    \
                page-search.c                   \
                page-static.c                   \
                page.c                          \
                paste.c                         \
                util.c
CORE_HDRS=      ${CORE_SRCS:.c=.h}
CORE_OBJS=      ${CORE_SRCS:.c=.o}
CORE_DEPS=      ${CORE_SRCS:.c=.d}
CORE_LIB=       libpaster.a

TESTS_SRCS=     tests/test-database.c
TESTS_OBJS=     ${TESTS_SRCS:.c=}

SQLITE_FLAGS=   -DSQLITE_THREADSAFE=0           \
                -DSQLITE_OMIT_LOAD_EXTENSION    \
                -DSQLITE_OMIT_DEPRECATED        \
                -DSQLITE_DEFAULT_FOREIGN_KEYS=1
SQLITE_LIB=     libsqlite3.a

MY_CFLAGS=      -std=c11                        \
                -I .                            \
                -I extern                       \
                -D_XOPEN_SOURCE=700             \
                -DSHAREDIR=\"${SHAREDIR}\"      \
                -DVARDIR=\"${VARDIR}\"          \
                `pkg-config --cflags kcgi-html`

MY_LDFLAGS=     `pkg-config --libs kcgi-html`

.SUFFIXES:
.SUFFIXES: .o .c .in

all: pasterd pasterd-clean paster

-include ${CORE_DEPS} paster.d pasterd-clean.d

.c.o:
	${CC} ${MY_CFLAGS} ${CFLAGS} -MMD -c $<

.c:
	${CC} ${MY_CFLAGS} ${CFLAGS} -MMD $< -o $@ ${CORE_LIB} ${SQLITE_LIB} ${MY_LDFLAGS} ${LDFLAGS}

.o:
	${CC} ${MY_CFLAGS} ${CFLAGS} -MMD $< -o $@ ${CORE_LIB} ${SQLITE_LIB} ${MY_LDFLAGS} ${LDFLAGS}

.in:
	sed -e "s|@SHAREDIR@|${SHAREDIR}|" \
	    -e "s|@VARDIR@|${VARDIR}|" \
	    < $< > $@

${SQLITE_LIB}: extern/sqlite3.c extern/sqlite3.h
	${CC} ${CFLAGS} ${SQLITE_FLAGS} -c extern/sqlite3.c -o extern/sqlite3.o
	${AR} -rc $@ extern/sqlite3.o

${CORE_LIB}: ${CORE_OBJS}
	${AR} -rc $@ ${CORE_OBJS}

pasterd.o: ${CORE_LIB} ${SQLITE_LIB} pasterd.8

pasterd-clean.o: ${CORE_LIB} ${SQLITE_LIB} pasterd-clean.8

paster: paster.sh paster.1
	cp paster.sh paster
	chmod +x paster

clean:
	rm -f ${SQLITE_LIB} extern/sqlite3.o
	rm -f ${CORE_LIB} ${CORE_OBJS} ${CORE_DEPS}
	rm -f pasterd pasterd.d pasterd.o pasterd.8
	rm -f pasterd-clean pasterd-clean.d pasterd-clean.o pasterd-clean.8
	rm -f paster paster.1
	rm -f test.db ${TESTS_OBJS}

install-paster:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	cp paster ${DESTDIR}${BINDIR}
	chmod 755 ${DESTDIR}${BINDIR}/paster
	cp paster.1 ${DESTDIR}${MANDIR}/man1/paster.1

install-pasterd:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man8
	cp pasterd ${DESTDIR}${BINDIR}
	cp pasterd-clean ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${SHAREDIR}/paster
	cp -R themes ${DESTDIR}${SHAREDIR}/paster
	cp pasterd.8 ${DESTDIR}${MANDIR}/man8
	cp pasterd-clean.8 ${DESTDIR}${MANDIR}/man8

install: install-pasterd install-paster

dist:
	mkdir -p paster-${VERSION}
	cp -R extern paster-${VERSION}
	cp -R themes paster-${VERSION}
	cp -R tests paster-${VERSION}
	cp ${CORE_SRCS} ${CORE_HDRS} paster-${VERSION}
	cp pasterd.8.in pasterd.c paster-${VERSION}
	cp pasterd-clean.8.in pasterd-clean.c paster-${VERSION}
	cp paster.1.in paster.sh paster-${VERSION}
	cp Makefile CHANGES.md CONTRIBUTE.md CREDITS.md INSTALL.md LICENSE.md \
	    README.md STYLE.md TODO.md paster-${VERSION}
	tar -cJf paster-${VERSION}.tar.xz paster-${VERSION}
	rm -rf paster-${VERSION}

${TESTS_OBJS}: ${CORE_LIB} ${SQLITE_LIB}

tests: ${TESTS_OBJS}
	for t in ${TESTS_OBJS}; do $$t; done

.PHONY: all clean dist run tests
