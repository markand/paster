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
#include "json-util.h"
#include "page-paste.h"
#include "page.h"
#include "util.h"

#include "html/paste.h"

static inline json_t *
mk_pagetitle(const json_t *paste)
{
	return json_sprintf("paster -- %s", ju_get_string(paste, "title"));
}

static inline json_t *
mk_date(const json_t *paste)
{
	return ju_date(ju_get_int(paste, "timestamp"));
}

static inline json_t *
mk_public(const json_t *paste)
{
	const intmax_t visible = ju_get_int(paste, "visible");

	return json_string(visible ? "Yes" : "No");
}

static inline json_t *
mk_expires(const json_t *paste)
{
	return ju_expires(
	    ju_get_int(paste, "timestamp"),
	    ju_get_int(paste, "duration")
	);
}

static void
get(struct kreq *req)
{
	json_t *paste;

	if (!(paste = database_get(req->path)))
		page_status(req, KHTTP_404);
	else {
		page(req, KHTTP_200, html_paste, ju_extend(paste, "{so so so so}",
			"pagetitle",    mk_pagetitle(paste),
			"date",         mk_date(paste),
			"public",       mk_public(paste),
			"expires",      mk_expires(paste)
		));
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
