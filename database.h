/*
 * database.h -- sqlite storage
 *
 * Copyright (c) 2020-2023 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PASTER_DATABASE_H
#define PASTER_DATABASE_H

#include <stddef.h>

struct paste;

struct database {
	void *handle;
};

/**
 * Global database handle for convenience.
 *
 * Should be initialized on startup.
 */
extern struct database database;

int
database_open(struct database *, const char *);

int
database_recents(struct database *, struct paste *, size_t *);

int
database_get(struct database *, struct paste *, const char *);

int
database_insert(struct database *, struct paste *);

int
database_search(struct database *,
                struct paste *,
                size_t *,
                const char *,
                const char *,
                const char *);

void
database_clear(struct database *);

void
database_finish(struct database *);

#endif /* !PASTER_DATABASE_H */
