/*
 * page.c -- page renderer
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

#include "config.h"
#include "page.h"
#include "util.h"

#include "html/footer.h"
#include "html/header.h"
#include "html/status.h"

#define CHAR(html) (const char *)(html)

enum {
	KEYWORD_TITLE,
};

struct page {
	struct kreq *req;
	struct ktemplate template;
	const char *title;
};

static const char *keywords[] = {
	[KEYWORD_TITLE] = "title"
};

static int
format(size_t keyword, void *data)
{
	struct page *page = data;

	switch (keyword) {
	case KEYWORD_TITLE:
		khttp_printf(page->req, "%s", page->title);
		break;
	default:
		break;
	}

	return 1;
}

void
page(struct kreq *req,
     enum khttp status,
     const char *title,
     const unsigned char *html,
     const struct ktemplate *tmpl)
{
	assert(req);
	assert(title);
	assert(html);
	assert(tmpl);

	struct page self = {
		.req = req,
		.template = {
			.cb = format,
			.arg = &self,
			.key = keywords,
			.keysz = NELEM(keywords)
		},
		.title = title,
	};

	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_body(req);
	khttp_template_buf(req, &self.template, CHAR(html_header), strlen(CHAR(html_header)));
	khttp_template_buf(req, tmpl, CHAR(html), strlen(CHAR(html)));
	khttp_template_buf(req, NULL, CHAR(html_footer), strlen(CHAR(html_footer)));
	khttp_free(req);
}
