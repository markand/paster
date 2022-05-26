/*
 * page-index.c -- page /
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
#include <stdint.h>

#include <kcgi.h>

#include "database.h"
#include "fragment-paste.h"
#include "page-index.h"
#include "page.h"
#include "paste.h"
#include "util.h"

struct template {
	struct kreq *req;
	const struct paste *pastes;
	size_t pastesz;
};

static const char *keywords[] = {
	"pastes"
};

static int
template(size_t keyword, void *arg)
{
	struct template *tp = arg;

	switch (keyword) {
	case 0:
		for (size_t i = 0; i < tp->pastesz; ++i)
			fragment_paste(tp->req, &tp->pastes[i]);
		break;
	default:
		break;
	}

	return 1;
}

static void
get(struct kreq *r)
{
	struct paste pastes[10] = {0};
	size_t pastesz = NELEM(pastes);

	if (!database_recents(pastes, &pastesz))
		page(r, NULL, KHTTP_500, "pages/500.html", "500");
	else
		page_index_render(r, pastes, pastesz);

	for (size_t i = 0; i < pastesz; ++i)
		paste_finish(&pastes[i]);
}

void
page_index_render(struct kreq *r, const struct paste *pastes, size_t pastesz)
{
	struct template data = {
		.req = r,
		.pastes = pastes,
		.pastesz = pastesz
	};

	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.arg = &data,
		.cb = template
	};

	page(r, &kt, KHTTP_200, "pages/index.html", "Recent pastes");
}

void
page_index(struct kreq *r)
{
	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	default:
		page(r, NULL, KHTTP_400, "400.html", "400");
		break;
	}
}
