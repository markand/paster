/*
 * page-fork.c -- page /fork/<id>
 *
 * Copyright (c) 2020 David Demelier <markand@malikania.fr>
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
#include <string.h>

#include <kcgi.h>

#include "database.h"
#include "page-new.h"
#include "page.h"
#include "paste.h"

static void
get(struct kreq *req)
{
	struct paste paste = {0};

	if (!database_get(&paste, req->path))
		page(req, NULL, KHTTP_404, "404.html");
	else {
		page_new_render(req, &paste);
		paste_finish(&paste);
	}
}

void
page_fork(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		get(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}
