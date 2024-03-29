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

-include config.mk

# User options.
CC ?=           cc
CFLAGS ?=       -DNDEBUG -O3

# Installation paths.
PREFIX ?=       /usr/local
BINDIR ?=       $(PREFIX)/bin
SHAREDIR ?=     $(PREFIX)/share
MANDIR ?=       $(PREFIX)/share/man
VARDIR ?=       $(PREFIX)/var

# External libraries
KCGI_INCS :=    $(shell pkg-config --cflags kcgi kcgi-html)
KCGI_LIBS :=    $(shell pkg-config --libs kcgi kcgi-html)

# No user options below this line.

VERSION :=              0.3.0

LIBPASTER_SRCS +=       extern/libsqlite/sqlite3.c
LIBPASTER_SRCS +=       config.c
LIBPASTER_SRCS +=       database.c
LIBPASTER_SRCS +=       http.c
LIBPASTER_SRCS +=       log.c
LIBPASTER_SRCS +=       page-download.c
LIBPASTER_SRCS +=       page-fork.c
LIBPASTER_SRCS +=       page-index.c
LIBPASTER_SRCS +=       page-new.c
LIBPASTER_SRCS +=       page-paste.c
LIBPASTER_SRCS +=       page-search.c
LIBPASTER_SRCS +=       page-static.c
LIBPASTER_SRCS +=       page-status.c
LIBPASTER_SRCS +=       page.c
LIBPASTER_SRCS +=       paste.c
LIBPASTER_SRCS +=       util.c
LIBPASTER_OBJS :=       $(LIBPASTER_SRCS:.c=.o)
LIBPASTER_DEPS :=       $(LIBPASTER_SRCS:.c=.d)
LIBPASTER :=            libpaster.a

LIBPASTER_SQL_SRCS :=   sql/clear.sql
LIBPASTER_SQL_SRCS +=   sql/get.sql
LIBPASTER_SQL_SRCS +=   sql/init.sql
LIBPASTER_SQL_SRCS +=   sql/insert.sql
LIBPASTER_SQL_SRCS +=   sql/recents.sql
LIBPASTER_SQL_SRCS +=   sql/search.sql
LIBPASTER_SQL_OBJS :=   $(LIBPASTER_SQL_SRCS:.sql=.h)

TESTS_SRCS :=           tests/test-database.c
TESTS_OBJS :=           $(TESTS_SRCS:.c=.o)
TESTS :=                $(TESTS_SRCS:.c=)

override CFLAGS +=      -DSQLITE_DEFAULT_FOREIGN_KEYS=1
override CFLAGS +=      -DSQLITE_OMIT_DEPRECATED
override CFLAGS +=      -DSQLITE_OMIT_LOAD_EXTENSION
override CFLAGS +=      -DSQLITE_THREADSAFE=0
override CFLAGS +=      -DSHAREDIR=\"$(SHAREDIR)\"
override CFLAGS +=      -DVARDIR=\"$(VARDIR)\"
override CFLAGS +=      -I.
override CFLAGS +=      -Iextern
override CFLAGS +=      -Iextern/libsqlite
override CFLAGS +=      $(KCGI_INCS)

override CPPFLAGS :=    -MMD

SED :=                  sed -e "s|@SHAREDIR@|$(SHAREDIR)|" \
                            -e "s|@VARDIR@|$(VARDIR)|"
BCC :=                  extern/bcc/bcc

all: pasterd paster

-include $(LIBPASTER_DEPS)

%.h: %.html
	$(BCC) -cs0 $< html_${<F} > $@

%.h: %.sql
	$(BCC) -cs0 $< sql_${<F} > $@

%: %.sh
	$(SED) < $< > $@

%.a:
	$(AR) -rc $@ $^

$(LIBPASTER_SQL_OBJS): extern/bcc/bcc
$(LIBPASTER_SRCS): $(LIBPASTER_SQL_OBJS)
$(LIBPASTER): $(LIBPASTER_OBJS)

pasterd: private LDLIBS += $(KCGI_LIBS) -lpthread
pasterd: $(LIBPASTER)

clean:
	rm -f extern/bcc/bcc extern/bcc/bcc.d
	rm -f $(LIBPASTER) $(LIBPASTER_OBJS) $(LIBPASTER_DEPS) $(LIBPASTER_SQL_OBJS)
	rm -f paster pasterd pasterd.d
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

install: install-pasterd install-paster

$(TESTS_OBJS): $(CORE_LIB)

tests: $(TESTS)
	for t in $(TESTS); do $$t; done

.PHONY: all clean tests
