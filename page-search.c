/*
 * page-search.c -- page /search
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
#include <string.h>

#include "database.h"
#include "json-util.h"
#include "page-index.h"
#include "page-search.h"
#include "page.h"
#include "util.h"

#include "html/search.h"

#define TITLE    "paster -- search"
#define LIMIT    16

static void
get(struct kreq *req)
{
	page(req, KHTTP_200, html_search, json_pack("{ss so}",
		"pagetitle",    "paster -- search",
		"languages",    ju_languages(NULL)
	));
}

static void
post(struct kreq *req)
{
	json_t *pastes;
	const char *key, *val, *title = NULL, *author = NULL, *language = NULL;

	for (size_t i = 0; i < req->fieldsz; ++i) {
		key = req->fields[i].key;
		val = req->fields[i].val;

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

	if (!(pastes = database_search(16, title, author, language)))
		page_status(req, KHTTP_500);
	else
		page_index_render(req, pastes);
}

void
page_search(struct kreq *req)
{
	assert(req);

	switch (req->method) {
	case KMETHOD_GET:
		get(req);
		break;
	case KMETHOD_POST:
		post(req);
		break;
	default:
		page_status(req, KHTTP_400);
		break;
	}
}
