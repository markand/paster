/*
 * page-new.c -- page /new
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
#include <stdlib.h>

#include "database.h"
#include "page-new.h"
#include "page-status.h"
#include "page.h"
#include "paste.h"
#include "util.h"

#define TITLE "paster -- create a new paste"

enum {
	KEYWORD_TITLE,
	KEYWORD_AUTHOR,
	KEYWORD_LANGUAGES,
	KEYWORD_DURATIONS,
	KEYWORD_CODE
};

struct page {
	struct kreq *req;
	struct ktemplate template;
	const struct paste *paste;
};

static const char * const keywords[] = {
	[KEYWORD_TITLE]         = "title",
	[KEYWORD_AUTHOR]        = "author",
	[KEYWORD_LANGUAGES]     = "languages",
	[KEYWORD_DURATIONS]     = "durations",
	[KEYWORD_CODE]          = "code"
};

static long long int
duration(const char *val)
{
	for (size_t i = 0; i < durationsz; ++i)
		if (strcmp(val, durations[i].title) == 0)
			return durations[i].secs;

	/* Default to day. */
	return 60 * 60 * 24;
}

static int
format(size_t kw, void *data)
{
	struct page *page = data;
	struct khtmlreq html;

	khtml_open(&html, page->req, KHTML_PRETTY);

	switch (kw) {
	case KEYWORD_TITLE:
		if (page->paste)
			khtml_printf(&html, page->paste->title);
		break;
	case KEYWORD_AUTHOR:
		if (page->paste)
			khtml_printf(&html, page->paste->author);
		break;
	case KEYWORD_LANGUAGES:
		for (size_t i = 0; i < languagesz; ++i) {
			if (page->paste && strcmp(page->paste->language, languages[i]) == 0)
				khtml_attr(&html, KELEM_OPTION,
				    KATTR_VALUE, languages[i],
				    KATTR_SELECTED, "selected",
				    KATTR__MAX
				);
			else
				khtml_attr(&html, KELEM_OPTION, KATTR_VALUE, languages[i], KATTR__MAX);
			khtml_printf(&html, "%s", languages[i]);
			khtml_closeelem(&html, 1);
		}
		break;
	case KEYWORD_DURATIONS:
		for (size_t i = 0; i < durationsz; ++i) {
			khtml_attr(&html, KELEM_OPTION, KATTR_VALUE, durations[i].title, KATTR__MAX);
			khtml_printf(&html, "%s", durations[i].title);
			khtml_closeelem(&html, 1);
		}
		break;
	case KEYWORD_CODE:
		if (page->paste)
			khtml_puts(&html, page->paste->code);
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
	page_new_render(r, NULL);
}

static void
post(struct kreq *req)
{
	struct paste paste;
	const char *key, *val, *scheme;
	int raw = 0;

	paste_init(&paste);

	// TODO: add verification support.
	for (size_t i = 0; i < req->fieldsz; ++i) {
		key = req->fields[i].key;
		val = req->fields[i].val;

		if (strcmp(key, "title") == 0 && strlen(val))
			replace(&paste.title, val);
		else if (strcmp(key, "author") == 0 && strlen(val))
			replace(&paste.author, val);
		else if (strcmp(key, "language") == 0)
			replace(&paste.language, val);
		else if (strcmp(key, "duration") == 0)
			paste.duration = duration(val);
		else if (strcmp(key, "code") == 0)
			replace(&paste.code, val);
		else if (strcmp(key, "visible") == 0)
			paste.visible = strcmp(val, "on") == 0;
		else if (strcmp(key, "raw") == 0)
			raw = strcmp(val, "on") == 0;
	}

	if (database_insert(&paste) < 0)
		page_status(req, KHTTP_500);
	else {
		scheme = req->scheme == KSCHEME_HTTP ? "http" : "https";

		if (raw) {
			/* For CLI users (e.g. paster) just print the location. */
			khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_201]);
			khttp_body(req);
			khttp_printf(req, "%s://%s/paste/%s\n", scheme, req->host, paste.id);
		} else {
			/* Otherwise, redirect to paste details. */
			khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
			khttp_head(req, kresps[KRESP_LOCATION], "/paste/%s", paste.id);
			khttp_body(req);
		}

		khttp_free(req);
	}

	paste_finish(&paste);
}

void
page_new_render(struct kreq *req, const struct paste *paste)
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
		.paste = paste
	};

	page(req, KHTTP_200, TITLE, "new.html", &self.template);
}

void
page_new(struct kreq *req)
{
	assert(req);

	switch (req->method) {
	case KMETHOD_GET:
		get(req);
		break;
	case KMETHOD_POST:
		post(req);
		break;
	default:
		break;
	}
}
