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
#include "page-index.h"
#include "page-status.h"
#include "page.h"
#include "paste.h"
#include "util.h"

#define TITLE   "paster -- recent pastes"
#define HTML    "index.html"
#define LIMIT   16

struct page {
	struct kreq *req;
	struct ktemplate template;
	const struct paste *pastes;
	const size_t pastesz;
};

enum {
	KEYWORD_PASTES
};

static const char * const keywords[] = {
	[KEYWORD_PASTES] = "pastes"
};

static int
format(size_t keyword, void *data)
{
	struct page *page = data;
	struct khtmlreq html;
	const struct paste *paste;

	khtml_open(&html, page->req, KHTML_PRETTY);

	switch (keyword) {
	case KEYWORD_PASTES:
		for (size_t i = 0; i < page->pastesz; ++i) {
			paste = &page->pastes[i];

			khtml_elem(&html, KELEM_TR);

			/* link */
			khtml_elem(&html, KELEM_TD);
			khtml_attr(&html, KELEM_A,
			    KATTR_HREF, bprintf("/paste/%s", paste->id), KATTR__MAX);
			khtml_printf(&html, paste->title);
			khtml_closeelem(&html, 1);

			/* author */
			khtml_elem(&html, KELEM_TD);
			khtml_puts(&html, paste->author);
			khtml_closeelem(&html, 1);

			/* language */
			khtml_elem(&html, KELEM_TD);
			khtml_puts(&html, paste->language);
			khtml_closeelem(&html, 1);

			/* date */
			khtml_elem(&html, KELEM_TD);
			khtml_puts(&html, bstrftime("%F %T", localtime(&paste->timestamp)));
			khtml_closeelem(&html, 1);

			/* expiration */
			khtml_elem(&html, KELEM_TD);
			khtml_puts(&html, ttl(paste->timestamp, paste->duration));
			khtml_closeelem(&html, 1);

			khtml_closeelem(&html, 1);

		}
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
	struct paste pastes[LIMIT];
	size_t pastesz = NELEM(pastes);

	if (database_recents(&database, pastes, &pastesz) < 0)
		page_status(req, KHTTP_500);
	else {
		page_index_render(req, pastes, pastesz);

		for (size_t i = 0; i < pastesz; ++i)
			paste_finish(&pastes[i]);
	}
}

void
page_index_render(struct kreq *req, const struct paste *pastes, size_t pastesz)
{
	assert(req);
	assert(pastes);

	struct page self = {
		.req = req,
		.template = {
			.cb = format,
			.arg = &self,
			.key = keywords,
			.keysz = NELEM(keywords)
		},
		.pastes = pastes,
		.pastesz = pastesz
	};

	page(req, KHTTP_200, TITLE, HTML, &self.template);
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
