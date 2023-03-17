/*
 * page-status.c -- error page
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

#include "page.h"
#include "util.h"

enum {
	KEYWORD_CODE,
	KEYWORD_MESSAGE
};

struct page {
	struct kreq *req;
	struct ktemplate template;
	enum khttp status;
};

static const int status_codes[] = {
	[KHTTP_200]             = 200,
	[KHTTP_400]             = 400,
	[KHTTP_404]             = 404,
	[KHTTP_500]             = 500
};

static const char * const status_messages[] = {
	[KHTTP_200]             = "OK",
	[KHTTP_400]             = "Bad Request",
	[KHTTP_404]             = "Not Found",
	[KHTTP_500]             = "Internal Server Error"
};

static const char *keywords[] = {
	[KEYWORD_CODE]          = "code",
	[KEYWORD_MESSAGE]       = "message"
};

static int
format(size_t keyword, void *data)
{
	struct page *page = data;

	switch (keyword) {
	case KEYWORD_CODE:
		khttp_printf(page->req, "%d", status_codes[page->status]);
		break;
	case KEYWORD_MESSAGE:
		khttp_printf(page->req, "%s", status_messages[page->status]);
		break;
	default:
		break;
	}

	return 1;
}

void
page_status(struct kreq *req, enum khttp status)
{
	assert(req);

	struct page self = {
		.req = req,
		.template = {
			.cb = format,
			.arg = &self,
			.key = keywords,
			.keysz = NELEM(keywords)
		},
		.status = status
	};

	page(req, status, "paster -- error", "status.html", &self.template);
}
