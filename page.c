/*
 * page.c -- page renderer
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

#include "page.h"
#include "util.h"

struct template {
	struct kreq *req;
	const char *title;
};

static const char * const keywords[] = {
	"title"
};

static int
template(size_t keyword, void *arg)
{
	struct template *tp = arg;

	switch (keyword) {
	case 0:
		khttp_printf(tp->req, "%s", tp->title);
		break;
	default:
		break;
	}

	return 1;
}

void
page(struct kreq *req, const struct ktemplate *tmpl, enum khttp status, const char *file, const char *title)
{
	struct template data = {
		.req = req,
		.title = title
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.arg = &data,
		.cb = template
	};

	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_body(req);
	khttp_template(req, &kt, path("fragments/header.html"));
	khttp_template(req, tmpl, path(file));
	khttp_template(req, NULL, path("fragments/footer.html"));
	khttp_free(req);
}
