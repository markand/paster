/*
 * fmt-paste.h -- page formatter for pastes
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

#ifndef PASTER_FMT_PASTE_H
#define PASTER_FMT_PASTE_H

#include <stddef.h>

struct kreq;
struct paste;

struct fmt_paste_vec {
	const struct paste *pastes;
	size_t pastesz;
};

void
fmt_paste(struct kreq *, const struct paste *);

void
fmt_pastes(struct kreq *, const struct fmt_paste_vec *);

void
fmt_paste_table(struct kreq *, const struct fmt_paste_vec *);

#endif /* !PASTER_FMT_PASTE_H */
