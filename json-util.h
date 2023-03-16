/*
 * json-util.h -- utilities for JSON
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

#ifndef PASTER_PASTER_JSON_UTIL_H
#define PASTER_PASTER_JSON_UTIL_H

#include <jansson.h>

struct paste;

/**
 * Create an array of all possible languages supported by the application. If
 * the selected argument is not null it will also add a JSON property selected
 * (mostly used when rendering an existing paste).
 *
 * Example of generated schema:
 *
 * ```javascript
 * [
 *   {
 *     "name": "nohighlight"
 *   }
 *   {
 *     "name": "c",
 *     "selected": "selected"
 *   }
 *   {
 *     "name": "cpp"
 *   }
 * ]
 * ```
 *
 * \param selected the current selected language (or NULL if none)
 * \return a JSON array of objects
 */
json_t *
ju_languages(const char *selected);

/**
 * Create a list of duration in the form:
 *
 * ```javascript
 * [
 *   {
 *     "value": "day"
 *   }
 *   {
 *     "value": "hour"
 *   }
 * ]
 * ```
 *
 * \return a JSON array of objects
 */
json_t *
ju_durations(void);

/**
 * Create a convenient ISO date string containing the paste creation date.
 *
 * \pre paste != NULL
 * \param paste this paste
 * \return a string with an ISO date
 */
json_t *
ju_date(const struct paste *paste);

/**
 * Create a convenient remaining time for the given paste.
 *
 * Returns strings in the form:
 *
 * - `2 day(s)`
 * - `3 hours(s)`
 *
 * \pre paste != NULL
 * \param paste this paste
 * \return a string containing the expiration time
 */
json_t *
ju_expiration(const struct paste *paste);

#endif /* !PASTER_PASTER_JSON_UTIL_H */
