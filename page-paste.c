/*
 * page-paste.c -- page /paste/<id>
 *
 * Copyright (c) 2020-2022 David Demelier <markand@malikania.fr>
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

#include <kcgi.h>
#include <kcgihtml.h>

#include "database.h"
#include "paste.h"
#include "page-paste.h"
#include "page.h"
#include "util.h"

struct template {
	struct kreq *req;
	struct paste *paste;
};

static const char *keywords[] = {
	"author",
	"code",
	"date",
	"expiration",
	"id",
	"language",
	"title",
	"visible"
};

static int
template(size_t keyword, void *arg)
{
	const struct template *tp = arg;
	struct khtmlreq html;

	khtml_open(&html, tp->req, KHTML_PRETTY);

	switch (keyword) {
	case 0:
		khtml_puts(&html, tp->paste->author);
		break;
	case 1:
		khtml_puts(&html, tp->paste->code);
		break;
	case 2:
		khtml_puts(&html, bstrftime("%c", localtime(&tp->paste->timestamp)));
		break;
	case 3:
		khtml_puts(&html, ttl(tp->paste->timestamp, tp->paste->duration));
		break;
	case 4:
		khtml_puts(&html, tp->paste->id);
		break;
	case 5:
		khtml_puts(&html, tp->paste->language);
		break;
	case 6:
		khtml_puts(&html, tp->paste->title);
		break;
	case 7:
		khtml_puts(&html, bprintf(tp->paste->visible ? "Yes" : "No"));
		break;
	default:
		break;
	}

	khtml_close(&html);

	return 1;
}

static void
get(struct kreq *r)
{
	struct paste paste = {0};
	struct template data = {
		.req = r,
		.paste = &paste
	};

	if (!database_get(&paste, r->path))
		page(r, NULL, KHTTP_404, "pages/404.html");
	else {
		const struct ktemplate kt = {
			.key = keywords,
			.keysz = NELEM(keywords),
			.cb = template,
			.arg = &data
		};

		page(r, &kt, KHTTP_200, "pages/paste.html");
		paste_finish(&paste);
	}
}

void
page_paste(struct kreq *r)
{
	assert(r);

	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	default:
		page(r, NULL, KHTTP_400, "pages/400.html");
		break;
	}
}
