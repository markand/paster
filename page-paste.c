/*
 * page-paste.c -- page /paste/<id>
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
#include "paste.h"
#include "page-paste.h"
#include "page.h"
#include "util.h"

#include "html/paste.h"

// TODO: share this.
static inline json_t *
create_date(const struct paste *paste)
{
	return json_string(bstrftime("%c", localtime(&paste->timestamp)));
}

static inline json_t *
create_expiration(const struct paste *paste)
{
	return json_string(ttl(paste->timestamp, paste->duration));
}

static inline json_t *
create_pagetitle(const struct paste *paste)
{
	return json_sprintf("paster -- %s", paste->title);
}

static inline json_t *
create_paste(const struct paste *paste)
{
	return json_pack("{so ss ss ss ss ss ss so so}",
		"pagetitle",    create_pagetitle(paste),
		"id",           paste->id,
		"title",        paste->title,
		"author",       paste->author,
		"language",     paste->language,
		"code",         paste->code,
		"public",       paste->visible ? "Yes" : "No",
		"date",         create_date(paste),
		"expiration",   create_expiration(paste)
	);
}

static void
get(struct kreq *r)
{
	struct paste paste = {0};

	if (!database_get(&paste, r->path))
		page_status(r, KHTTP_404);
	else {
		page(r, KHTTP_200, html_paste, create_paste(&paste));
		paste_finish(&paste);
	}
}

void
page_paste(struct kreq *req)
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
