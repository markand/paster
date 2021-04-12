/*
 * page-search.c -- page /search
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

#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kcgi.h>

#include "database.h"
#include "fragment-language.h"
#include "page-index.h"
#include "page-search.h"
#include "page.h"
#include "paste.h"
#include "util.h"

static const char *keywords[] = {
	"languages"
};

static int
template(size_t keyword, void *arg)
{
	struct kreq *r = arg;

	switch (keyword) {
	case 0:
		for (size_t i = 0; i < languagesz; ++i)
			fragment_language(r, languages[i], false);
		break;
	default:
		break;
	}

	return 1;
}

static void
get(struct kreq *r)
{
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.cb = template,
		.arg = r
	};

	page(r, &kt, KHTTP_200, "pages/search.html");
}

static void
post(struct kreq *r)
{
	struct paste pastes[10] = {0};
	size_t pastesz = NELEM(pastes);
	const char *title = NULL;
	const char *author = NULL;
	const char *language = NULL;

	for (size_t i = 0; i < r->fieldsz; ++i) {
		const char *key = r->fields[i].key;
		const char *val = r->fields[i].val;

		if (strcmp(key, "title") == 0)
			title = val;
		else if (strcmp(key, "author") == 0)
			author = val;
		else if (strcmp(key, "language") == 0)
			language = val;
	}

	/* Sets to null if they are empty. */
	if (title && strlen(title) == 0)
		title = NULL;
	if (author && strlen(author) == 0)
		author = NULL;

	if (!database_search(pastes, &pastesz, title, author, language))
		page(r, NULL, KHTTP_500, "pages/500.html");
	else
		page_index_render(r, pastes, pastesz);

	for (size_t i = 0; i < pastesz; ++i)
		paste_finish(&pastes[i]);
}

void
page_search(struct kreq *r)
{
	assert(r);

	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	case KMETHOD_POST:
		post(r);
		break;
	default:
		page(r, NULL, KHTTP_400, "pages/400.html");
		break;
	}
}
