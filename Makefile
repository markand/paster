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
CFLAGS=         -DNDEBUG -O3

# Installation paths.
PREFIX=         /usr/local
BINDIR=         $(PREFIX)/bin
SHAREDIR=       $(PREFIX)/share
MANDIR=         $(PREFIX)/share/man
VARDIR=         $(PREFIX)/var

VERSION=        0.3.0

CORE_SRCS=      config.c                        \
                database.c                      \
                fmt.c                           \
                fmt-paste.c                     \
                fragment-duration.c             \
                fragment-language.c             \
                fragment-paste.c                \
                fragment.c                      \
                http.c                          \
                log.c                           \
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
CORE_OBJS=      $(CORE_SRCS:.c=.o)
CORE_LIB=       libpaster.a

HTML_SRCS=      html/footer.html                \
                html/header.html                \
                html/index.html                 \
                html/paste-table.html           \
                html/paste.html
HTML_OBJS=      $(HTML_SRCS:.html=.h)

TESTS_SRCS=     tests/test-database.c
TESTS_OBJS=     $(TESTS_SRCS:.c=.o)
TESTS=          $(TESTS_SRCS:.c=)

SQLITE_FLAGS=   -DSQLITE_THREADSAFE=0           \
                -DSQLITE_OMIT_LOAD_EXTENSION    \
                -DSQLITE_OMIT_DEPRECATED        \
                -DSQLITE_DEFAULT_FOREIGN_KEYS=1
SQLITE_LIB=     libsqlite3.a

KCGI_INCS=      `pkg-config --cflags kcgi kcgi-html`
KCGI_LIBS=      `pkg-config --libs kcgi kcgi-html`

INCS=           -I. -Iextern $(KCGI_INCS)
DEFS=           -D_POSIX_C_SOURCE=200809L -DVARDIR=\"$(VARDIR)\" -DSHAREDIR=\"$(SHAREDIR)\"
LIBS=           $(KCGI_LIBS)
SED=            sed -e "s|@SHAREDIR@|$(SHAREDIR)|" \
                    -e "s|@VARDIR@|$(VARDIR)|"
BCC=            extern/bcc/bcc

.SUFFIXES:
.SUFFIXES: .o .c .h .sh .html

all: pasterd pasterd-clean paster

.c.o:
	$(CC) $(INCS) $(DEFS) $(CFLAGS) -c $< -o $@

.o:
	$(CC) $(INCS) $(DEFS) $(CFLAGS) $< -o $@ $(CORE_LIB) $(SQLITE_LIB) $(LIBS) $(LDFLAGS)

.html.h:
	$(BCC) -cs0 $< html_${<F} > $@

.sh:
	$(SED) < $< > $@

$(SQLITE_LIB): extern/sqlite3.c extern/sqlite3.h
	$(CC) $(CFLAGS) $(SQLITE_FLAGS) -c extern/sqlite3.c -o extern/sqlite3.o
	$(AR) -rc $@ extern/sqlite3.o

$(HTML_OBJS): $(BCC)

$(CORE_OBJS): $(HTML_OBJS)

$(CORE_LIB): $(CORE_OBJS)
	$(AR) -rc $@ $(CORE_OBJS)

paster: paster.sh
	cp paster.sh paster
	chmod +x paster

pasterd: $(CORE_LIB) $(SQLITE_LIB)

clean:
	rm -f $(SQLITE_LIB) extern/sqlite3.o
	rm -f $(CORE_LIB) $(CORE_OBJS)
	rm -f paster pasterd-clean pasterd
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
	cp pasterd-clean $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(SHAREDIR)/paster
	cp -R themes $(DESTDIR)$(SHAREDIR)/paster
	$(SED) < pasterd.8 > $(DESTDIR)$(MANDIR)/man8/pasterd.8
	$(SED) < pasterd-clean.8 > $(DESTDIR)$(MANDIR)/man8/pasterd-clean.8
	$(SED) < pasterd-themes.5 > $(DESTDIR)$(MANDIR)/man5/pasterd-themes.5

install: install-pasterd install-paster

$(TESTS_OBJS): $(CORE_LIB) $(SQLITE_LIB)

tests: $(TESTS)
	for t in $(TESTS); do $$t; done

.PHONY: all clean tests
