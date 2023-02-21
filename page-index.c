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
#include "fmt-paste.h"
#include "fmt.h"
#include "fragment-paste.h"
#include "page-index.h"
#include "page.h"
#include "paste.h"
#include "util.h"

#include "html/index.h"

static void
print_paste_table(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)html;

	fmt_paste_table(req, data);
}

static void
get(struct kreq *r)
{
	struct paste pastes[10] = {0};
	size_t pastesz = NELEM(pastes);

	if (!database_recents(pastes, &pastesz))
		page(r, NULL, KHTTP_500, "pages/500.html", "500");
	else {
		page_index_render(r, pastes, pastesz);

		for (size_t i = 0; i < pastesz; ++i)
			paste_finish(&pastes[i]);
	}
}

void
page_index_render(struct kreq *r, const struct paste *pastes, size_t pastesz)
{
	assert(r);
	assert(pastes);

	struct fmt_paste_vec vec = {
		.pastes = pastes,
		.pastesz = pastesz
	};

	page2(r, KHTTP_200, "recent pastes", html_index, &vec, (const struct fmt_printer []) {
		{ "paste-table",        print_paste_table       },
		{ NULL,                 NULL                    }
	});
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
