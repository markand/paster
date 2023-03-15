#
# Makefile -- basic makefile for paster
#
# Copyright (c) 2020-2023 David Demelier <markand@malikania.fr>
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
BINDIR=         $(PREFIX)/bin
SHAREDIR=       $(PREFIX)/share
MANDIR=         $(PREFIX)/share/man
VARDIR=         $(PREFIX)/var

-include config.mk

VERSION=        0.3.0

CORE_SRCS=      extern/libmustach/mustach-jansson.c     \
                extern/libmustach/mustach-wrap.c        \
                extern/libmustach/mustach.c             \
                extern/libmustach/mustach.c             \
                extern/libsqlite/sqlite3.c              \
                config.c                                \
                database.c                              \
                http.c                                  \
                log.c                                   \
                page-download.c                         \
                page-fork.c                             \
                page-index.c                            \
                page-new.c                              \
                page-paste.c                            \
                page-search.c                           \
                page-static.c                           \
                page.c                                  \
                paste.c                                 \
                util.c
CORE_OBJS=      $(CORE_SRCS:.c=.o)
CORE_DEPS=      $(CORE_SRCS:.c=.d)
CORE_LIB=       libpaster.a

HTML_SRCS=      html/footer.html                \
                html/header.html                \
                html/index.html                 \
                html/new.html                   \
                html/paste.html                 \
                html/search.html                \
                html/status.html
HTML_OBJS=      $(HTML_SRCS:.html=.h)

TESTS_SRCS=     tests/test-database.c
TESTS_OBJS=     $(TESTS_SRCS:.c=.o)
TESTS=          $(TESTS_SRCS:.c=)

KCGI_INCS=      `pkg-config --cflags kcgi`
KCGI_LIBS=      `pkg-config --libs kcgi`

JANSSON_INCS=   `pkg-config --cflags jansson`
JANSSON_LIBS=   `pkg-config --libs jansson`

DEFS=           -DSQLITE_DEFAULT_FOREIGN_KEYS=1 \
                -DSQLITE_OMIT_DEPRECATED \
                -DSQLITE_OMIT_LOAD_EXTENSION \
                -DSQLITE_THREADSAFE=0 \
                -DSHAREDIR=\"$(SHAREDIR)\" \
                -DVARDIR=\"$(VARDIR)\"
INCS=           -I. \
                -Iextern \
                -Iextern/libmustach \
                -Iextern/libsqlite \
                $(KCGI_INCS) \
                $(JANSSON_INCS)
LIBS=           $(KCGI_LIBS) \
                $(JANSSON_LIBS)

SED=            sed -e "s|@SHAREDIR@|$(SHAREDIR)|" \
                    -e "s|@VARDIR@|$(VARDIR)|"
BCC=            extern/bcc/bcc

.SUFFIXES:
.SUFFIXES: .c .o .h .sh .html

all: pasterd paster

-include $(CORE_DEPS)

.c.o:
	$(CC) $(INCS) $(DEFS) $(CFLAGS) -MMD -c $< -o $@

.c:
	$(CC) $(INCS) $(DEFS) $(CFLAGS) $< -o $@ $(CORE_LIB) $(SQLITE_LIB) $(LIBS) $(LDFLAGS)

.html.h:
	$(BCC) -cs0 $< html_${<F} > $@

.sh:
	$(SED) < $< > $@

$(HTML_OBJS): $(BCC)

$(CORE_SRCS): $(HTML_OBJS)

$(CORE_LIB): $(CORE_OBJS)
	$(AR) -rc $@ $(CORE_OBJS)

paster: paster.sh
	cp paster.sh paster
	chmod +x paster

pasterd: $(CORE_LIB)

clean:
	rm -f $(CORE_LIB) $(CORE_OBJS)
	rm -f paster pasterd
	rm -f test.db $(TESTS_OBJS)

install-paster:
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	cp paster $(DESTDIR)$(BINDIR)
	chmod 755 $(DESTDIR)$(BINDIR)/paster
	$(SED) < paster.1 > $(DESTDIR)$(MANDIR)/man1/paster.1

install-pasterd:
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)/man5
	mkdir -p $(DESTDIR)$(MANDIR)/man8
	cp pasterd $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(SHAREDIR)/paster
	cp -R themes $(DESTDIR)$(SHAREDIR)/paster
	$(SED) < pasterd.8 > $(DESTDIR)$(MANDIR)/man8/pasterd.8
	$(SED) < pasterd-themes.5 > $(DESTDIR)$(MANDIR)/man5/pasterd-themes.5

install: install-pasterd install-paster

$(TESTS_OBJS): $(CORE_LIB)

tests: $(TESTS)
	for t in $(TESTS); do $$t; done

.PHONY: all clean tests
