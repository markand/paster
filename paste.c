/*
 * paste.c -- paste definition
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "paste.h"
#include "util.h"

void
paste_init(struct paste *paste)
{
	assert(paste);

	memset(paste, 0, sizeof (*paste));
	paste->title = estrdup(PASTE_DEFAULT_TITLE);
	paste->author = estrdup(PASTE_DEFAULT_AUTHOR);
	paste->language = estrdup(PASTE_DEFAULT_LANGUAGE);
	paste->timestamp = time(NULL);
	paste->duration = PASTE_DURATION_DAY;
}

void
paste_finish(struct paste *paste)
{
	assert(paste);

	free(paste->id);
	free(paste->title);
	free(paste->author);
	free(paste->language);
	free(paste->code);
	memset(paste, 0, sizeof (struct paste));
}
