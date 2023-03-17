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
#include "page-paste.h"
#include "page-status.h"
#include "page.h"
#include "paste.h"
#include "util.h"

#define TITLE   "paster -- paste details"
#define HTML    "paste.html"

enum {
	KEYWORD_TITLE,
	KEYWORD_ID,
	KEYWORD_AUTHOR,
	KEYWORD_LANGUAGE,
	KEYWORD_DATE,
	KEYWORD_PUBLIC,
	KEYWORD_EXPIRES,
	KEYWORD_CODE
};

struct page {
	struct kreq *req;
	struct ktemplate template;
	struct paste paste;
};

static const char * const keywords[] = {
	[KEYWORD_TITLE]         = "title",
	[KEYWORD_ID]            = "id",
	[KEYWORD_AUTHOR]        = "author",
	[KEYWORD_LANGUAGE]      = "language",
	[KEYWORD_DATE]          = "date",
	[KEYWORD_PUBLIC]        = "public",
	[KEYWORD_EXPIRES]       = "expires",
	[KEYWORD_CODE]          = "code"
};

static int
format(size_t keyword, void *data)
{
	struct page *page = data;
	struct khtmlreq html;

	khtml_open(&html, page->req, 0);

	switch (keyword) {
	case KEYWORD_TITLE:
		khtml_puts(&html, page->paste.title);
		break;
	case KEYWORD_ID:
		khtml_puts(&html, page->paste.id);
		break;
	case KEYWORD_AUTHOR:
		khtml_puts(&html, page->paste.author);
		break;
	case KEYWORD_LANGUAGE:
		khtml_puts(&html, page->paste.language);
		break;
	case KEYWORD_DATE:
		khtml_puts(&html, bstrftime("%F %T", localtime(&page->paste.timestamp)));
		break;
	case KEYWORD_PUBLIC:
		khtml_puts(&html, page->paste.visible ? "Yes" : "No");
		break;
	case KEYWORD_EXPIRES:
		khtml_puts(&html, ttl(page->paste.timestamp, page->paste.duration));
		break;
	case KEYWORD_CODE:
		khtml_puts(&html, page->paste.code);
		break;
	default:
		break;
	}

	khtml_close(&html);

	return 1;
}

static void
get(struct kreq *req)
{
	struct page self = {
		.req = req,
		.template = {
			.cb = format,
			.arg = &self,
			.key = keywords,
			.keysz = NELEM(keywords)
		}
	};

	if (database_get(&database, &self.paste, req->path) < 0)
		page_status(req, KHTTP_404);
	else {
		page(req, KHTTP_200, TITLE, HTML, &self.template);
		paste_finish(&self.paste);
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
