/*
 * database.h -- sqlite storage
 *
 * Copyright (c) 2020-2021 David Demelier <markand@malikania.fr>
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

#include <stdbool.h>
#include <stddef.h>

struct paste;

bool
database_open(const char *);

bool
database_recents(struct paste *, size_t *);

bool
database_get(struct paste *, const char *);

bool
database_insert(struct paste *);

bool
database_search(struct paste *,
                size_t *,
                const char *,
                const char *,
                const char *);

void
database_clear(void);

void
database_finish(void);

#endif /* !PASTER_DATABASE_H */
