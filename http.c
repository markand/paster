/*
 * http.c -- HTTP parsing and rendering
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
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kcgi.h>

#include "config.h"
#include "database.h"
#include "http.h"
#include "log.h"
#include "paste.h"
#include "util.h"

static void page_index(struct kreq *);
static void page_new(struct kreq *);
static void page_fork(struct kreq *);
static void page_paste(struct kreq *);
static void page_about(struct kreq *);
static void page_download(struct kreq *);

enum page {
	PAGE_INDEX,
	PAGE_NEW,
	PAGE_FORK,
	PAGE_PASTE,
	PAGE_ABOUT,
	PAGE_DOWNLOAD,
	PAGE_LAST       /* Not used. */
};

static const char *pages[] = {
	[PAGE_INDEX]    = "",
	[PAGE_NEW]      = "new",
	[PAGE_FORK]     = "fork",
	[PAGE_PASTE]    = "paste",
	[PAGE_ABOUT]    = "about",
	[PAGE_DOWNLOAD] = "download",
};

static void (*handlers[])(struct kreq *req) = {
	[PAGE_INDEX]    = page_index,
	[PAGE_NEW]      = page_new,
	[PAGE_FORK]     = page_fork,
	[PAGE_PASTE]    = page_paste,
	[PAGE_ABOUT]    = page_about,
	[PAGE_DOWNLOAD] = page_download
};

struct tmpl_index {
	struct kreq *req;
	struct paste pastes[10];
	size_t count;
	size_t current;
};

struct tmpl_paste {
	struct kreq *req;
	struct paste paste;
};

static const char *
template(const char *filename)
{
	/* Build path to the template file. */
	static char path[PATH_MAX];

	snprintf(path, sizeof (path), "%s/%s", config.themedir, filename);

	return path;
}

static int
tmpl_paste(size_t index, void *arg)
{
	struct tmpl_paste *data = arg;
	struct paste *paste = &data->paste;

	switch (index) {
	case 0:
		khttp_puts(data->req, paste->uuid);
		break;
	case 1:
		khttp_puts(data->req, paste->title);
		break;
	case 2:
		khttp_puts(data->req, paste->author);
		break;
	case 3:
		khttp_puts(data->req, paste->language);
		break;
	case 4:
		khttp_puts(data->req, paste->code);
		break;
	case 5:
		/* TODO: timestamp here. */
		khttp_puts(data->req, "TODO");
		break;
	case 6:
		khttp_puts(data->req, bprintf("%s", paste->visible ? "Yes" : "No"));
		break;
	case 7:
		/* TODO: convert time left. */
		khttp_puts(data->req, "TODO");
		break;
	default:
		break;
	}

	return true;
}

static int
tmpl_index_pastes(size_t index, void *arg)
{
	struct tmpl_index *data = arg;
	struct paste *paste = &data->pastes[data->current];

	switch (index) {
	case 0:
		khttp_puts(data->req, paste->uuid);
		break;
	case 1:
		khttp_puts(data->req, paste->title);
		break;
	case 2:
		khttp_puts(data->req, paste->author);
		break;
	case 3:
		khttp_puts(data->req, paste->language);
		break;
	case 4:
		khttp_puts(data->req, bprintf("%d", paste->duration));
		break;
	default:
		break;
	}

	return true;
}

static int
tmpl_index(size_t index, void *arg)
{
	/* No check, only one index. */
	struct tmpl_index *data = arg;
	const char *keywords[] = {
		"uuid",
		"name",
		"author",
		"language",
		"expiration"
	};
	struct ktemplate kt = {
		.key    = keywords,
		.keysz  = 5,
		.arg    = data,
		.cb     = tmpl_index_pastes
	};

	for (size_t i = 0; i < data->count; ++i) {
		khttp_template(data->req, &kt, template("index-paste.html"));
		data->current++;
	}

	return true;
}

static void
header(struct kreq *req)
{
	khttp_template(req, NULL, template("header.html"));
}

static void
footer(struct kreq *req)
{
	khttp_template(req, NULL, template("footer.html"));
}

static void
page_index(struct kreq *req)
{
	struct tmpl_index data = {
		.req    = req,
		.count  = 10
	};

	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);

	if (!database_recents(data.pastes, &data.count)) {
		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_500]);
		khttp_body(req);
		khttp_template(req, NULL, template("500.html"));
	} else {
		const char *keywords[] = { "pastes" };
		struct ktemplate kt = {
			.key    = keywords,
			.keysz  = 1,
			.arg    = &data,
			.cb     = tmpl_index
		};

		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_body(req);
		header(req);
		khttp_template(req, &kt, template("index.html"));
		footer(req);
	}

	khttp_free(req);
}

static void
page_new(struct kreq *req)
{
	(void)req;
}

static void
page_fork(struct kreq *req)
{
	(void)req;
}

static void
page_paste(struct kreq *req)
{
	struct tmpl_paste data = {
		.req = req
	};

	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);

	if (!database_get(&data.paste, req->path)) {
		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_404]);
		khttp_body(req);
		khttp_template(req, NULL, template("404.html"));
	} else {
		const char *keywords[] = {
			"uuid",
			"title",
			"author",
			"language",
			"code",
			"timestamp",
			"visible",
			"duration"
		};
		const struct ktemplate kt = {
			.key    = keywords,
			.keysz  = 8,
			.cb     = tmpl_paste,
			.arg    = &data
		};

		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_body(req);
		header(req);
		khttp_template(req, &kt, template("paste.html"));
		footer(req);
		khttp_free(req);
	}
}

static void
page_about(struct kreq *req)
{
	(void)req;
}

static void
page_download(struct kreq *req)
{
	(void)req;
}

static void
process(struct kreq *req)
{
	assert(req);

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

void
http_cgi_run(void)
{
	struct kreq req;

	if (khttp_parse(&req, NULL, 0, pages, PAGE_LAST, 0) == KCGI_OK)
		process(&req);
}
