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

#include <jansson.h>

/**
 * Open the database specified by path.
 *
 * \pre path != NULL
 * \param path path to the SQLite file
 * \return 0 on success or -1 on error
 */
int
database_open(const char *path);

/**
 * Obtain a list of recent pastes.
 *
 * \param limit max number of items to fetch
 * \return a JSON array of paste objects or NULL on failure
 */
json_t *
database_recents(size_t limit);

/**
 * Obtain a specific paste by id.
 *
 * \pre id != NULL
 * \param id the paste identifier
 * \return NULL on failure or if not found
 */
json_t *
database_get(const char *id);

/**
 * Insert a new paste into the database.
 *
 * On insertion, the paste gets a new string "id" property generated from the
 * database.
 *
 * \pre paste != NULL
 * \param paste the paste object
 * \return 0 on success or -1 on failure
 */
int
database_insert(json_t *paste);

/**
 * Search for pastes based on criterias.
 *
 * If any of the criterias is NULL it is considered as ignored (and then match a
 * database item).
 *
 * \param limit max number of items to fetch
 * \param title paste title (or NULL to match any)
 * \param author paste author (or NULL to match any)
 * \param language paste language (or NULL to match any)
 * \return a JSON array of objects or NULL on failure
 */
json_t *
database_search(size_t limit,
                const char *title,
                const char *author,
                const char *language);

/**
 * Cleanup expired pastes.
 */
void
database_clear(void);

/**
 * Close the database handle
 */
void
database_finish(void);

#endif /* !PASTER_DATABASE_H */
