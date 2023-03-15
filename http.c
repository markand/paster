/*
 * http.c -- HTTP parsing and rendering
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

#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <kcgi.h>

#include "database.h"
#include "http.h"
#include "log.h"

#include "page-download.h"
#include "page-fork.h"
#include "page-index.h"
#include "page-new.h"
#include "page-paste.h"
#include "page-search.h"
#include "page-static.h"
#include "page.h"

enum page {
	PAGE_INDEX,
	PAGE_NEW,
	PAGE_FORK,
	PAGE_PASTE,
	PAGE_DOWNLOAD,
	PAGE_SEARCH,
	PAGE_STATIC,
	PAGE_LAST       /* Not used. */
};

static const char *pages[] = {
	[PAGE_INDEX]    = "",
	[PAGE_NEW]      = "new",
	[PAGE_FORK]     = "fork",
	[PAGE_PASTE]    = "paste",
	[PAGE_DOWNLOAD] = "download",
	[PAGE_SEARCH]   = "search",
	[PAGE_STATIC]   = "static"
};

static void (*handlers[])(struct kreq *req) = {
	[PAGE_INDEX]    = page_index,
	[PAGE_NEW]      = page_new,
	[PAGE_FORK]     = page_fork,
	[PAGE_PASTE]    = page_paste,
	[PAGE_DOWNLOAD] = page_download,
	[PAGE_SEARCH]   = page_search,
	[PAGE_STATIC]   = page_static
};

static void
process(struct kreq *req)
{
	assert(req);

	log_debug("http: accessing page '%s'", req->path);

	if (req->page == PAGE_LAST)
		page_status(req, KHTTP_404);
	else
		handlers[req->page](req);
}

void
http_fcgi_run(void)
{
	struct kreq req;
	struct kfcgi *fcgi;

	if (khttp_fcgi_init(&fcgi, NULL, 0, pages, PAGE_LAST, 0) != KCGI_OK)
		return;

	while (khttp_fcgi_parse(fcgi, &req) == KCGI_OK)
		process(&req);

	khttp_fcgi_free(fcgi);
}
