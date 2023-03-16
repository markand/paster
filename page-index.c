/*
 * page-index.c -- page /
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

#include "database.h"
#include "json-util.h"
#include "page-index.h"
#include "page.h"
#include "paste.h"
#include "util.h"

#include "html/index.h"

static void
get(struct kreq *r)
{
	struct paste pastes[10] = {0};
	size_t pastesz = NELEM(pastes);

	if (!database_recents(pastes, &pastesz))
		page_status(r, KHTTP_500);
	else {
		page_index_render(r, pastes, pastesz);

		for (size_t i = 0; i < pastesz; ++i)
			paste_finish(&pastes[i]);
	}
}

static inline json_t *
create_pastes(const struct paste *pastes, size_t pastesz)
{
	json_t *array = json_array();
	const struct paste *paste;

	for (size_t i = 0; i < pastesz; ++i) {
		paste = &pastes[i];

		json_array_append_new(array, json_pack("{ss ss ss ss so so}",
			"id",           paste->id,
			"author",       paste->author,
			"title",        paste->title,
			"date",         ju_date(paste),
			"expiration",   ju_expiration(paste)
		));
	}

	return array;
}

static inline json_t *
create_doc(const struct paste *pastes, size_t pastesz)
{
	return json_pack("{ss so}",
		"pagetitle",    "paster -- recent pastes",
		"pastes",       create_pastes(pastes, pastesz)
	);
}

void
page_index_render(struct kreq *req, const struct paste *pastes, size_t pastesz)
{
	assert(req);
	assert(pastes);

	page(req, KHTTP_200, html_index, create_doc(pastes, pastesz));
}

void
page_index(struct kreq *req)
{
	assert(req);

	switch (req->method) {
	case KMETHOD_GET:
		get(req);
		break;
	default:
		page_status(req, KHTTP_400);
		break;
	}
}
