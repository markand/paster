/*
 * paste.h -- paste definition
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

#ifndef PASTER_PASTE_H
#define PASTER_PASTE_H

#include <time.h>

#define PASTE_DURATION_HOUR      3600           /* Seconds in one hour. */
#define PASTE_DURATION_DAY       86400          /* Seconds in one day. */
#define PASTE_DURATION_WEEK      604800         /* Seconds in one week. */
#define PASTE_DURATION_MONTH     2592000        /* Rounded to 30 days. */

#define PASTE_DEFAULT_TITLE      "Untitled"
#define PASTE_DEFAULT_AUTHOR     "Anonymous"
#define PASTE_DEFAULT_LANGUAGE   "nohighlight"

struct paste {
	char *id;
	char *title;
	char *author;
	char *language;
	char *code;
	time_t timestamp;
	int visible;
	int duration;
};

void
paste_init(struct paste *paste);

void
paste_finish(struct paste *paste);

#endif /* !PASTER_PASTE_H */
