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
KCGI_INCS :=    $(shell pkg-config --cflags kcgi)
KCGI_LIBS :=    $(shell pkg-config --libs kcgi)

JANSSON_INCS := $(shell pkg-config --cflags jansson)
JANSSON_LIBS := $(shell pkg-config --libs jansson)

# No user options below this line.

VERSION :=              0.3.0

LIBPASTER_SRCS :=       extern/libmustach/mustach-jansson.c
LIBPASTER_SRCS +=       extern/libmustach/mustach-wrap.c
LIBPASTER_SRCS +=       extern/libmustach/mustach.c
LIBPASTER_SRCS +=       extern/libmustach/mustach.c
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
LIBPASTER_SRCS +=       page.c
LIBPASTER_SRCS +=       paste.c
LIBPASTER_SRCS +=       util.c
LIBPASTER_OBJS :=       $(LIBPASTER_SRCS:.c=.o)
LIBPASTER_DEPS :=       $(LIBPASTER_SRCS:.c=.d)
LIBPASTER :=            libpaster.a

LIBPASTER_HTML_SRCS :=  html/footer.html
LIBPASTER_HTML_SRCS +=  html/header.html
LIBPASTER_HTML_SRCS +=  html/index.html
LIBPASTER_HTML_SRCS +=  html/new.html
LIBPASTER_HTML_SRCS +=  html/paste.html
LIBPASTER_HTML_SRCS +=  html/search.html
LIBPASTER_HTML_SRCS +=  html/status.html
LIBPASTER_HTML_OBJS :=  $(LIBPASTER_HTML_SRCS:.html=.h)

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
override CFLAGS +=      -Iextern/libmustach
override CFLAGS +=      -Iextern/libsqlite
override CFLAGS +=      $(KCGI_INCS)
override CFLAGS +=      $(JANSSON_INCS)

override CPPFLAGS :=    -MMD

SED :=                  sed -e "s|@SHAREDIR@|$(SHAREDIR)|" \
                            -e "s|@VARDIR@|$(VARDIR)|"
BCC :=                  extern/bcc/bcc

all: pasterd paster

-include $(LIBPASTER_DEPS)

%.h: %.html
	$(BCC) -cs0 $< html_${<F} > $@

%: %.sh
	$(SED) < $< > $@

%.a:
	$(AR) -rc $@ $^

$(LIBPASTER_HTML_OBJS): extern/bcc/bcc
$(LIBPASTER_SRCS): $(LIBPASTER_HTML_OBJS)
$(LIBPASTER): $(LIBPASTER_OBJS)

pasterd: private LDLIBS += $(KCGI_LIBS) $(JANSSON_LIBS)
pasterd: $(LIBPASTER)

clean:
	rm -f extern/bcc/bcc
	rm -f $(LIBPASTER) $(LIBPASTER_OBJS) $(LIBPASTER_DEPS) $(LIBPASTER_HTML_OBJS)
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
