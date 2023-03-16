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

#include "database.h"
#include "json-util.h"
#include "page-new.h"
#include "page.h"
#include "util.h"

#include "html/new.h"

#define TITLE "paster -- create a new paste"

static long long int
duration(const char *val)
{
	for (size_t i = 0; i < durationsz; ++i)
		if (strcmp(val, durations[i].title) == 0)
			return durations[i].secs;

	/* Default to day. */
	return 60 * 60 * 24;
}

static void
get(struct kreq *r)
{
	page_new_render(r, NULL);
}

static void
post(struct kreq *req)
{
	const char *key, *val, *id, *scheme;
	json_t *paste;
	int raw = 0;

	paste = ju_paste_new();

	for (size_t i = 0; i < req->fieldsz; ++i) {
		key = req->fields[i].key;
		val = req->fields[i].val;

		if (strcmp(key, "title") == 0 && strlen(val))
			json_object_set_new(paste, "title", json_string(val));
		else if (strcmp(key, "author") == 0 && strlen(val))
			json_object_set_new(paste, "author", json_string(val));
		else if (strcmp(key, "language") == 0)
			json_object_set_new(paste, "language", json_string(val));
		else if (strcmp(key, "duration") == 0)
			json_object_set_new(paste, "duration", json_integer(duration(val)));
		else if (strcmp(key, "code") == 0)
			json_object_set_new(paste, "code", json_string(val));
		else if (strcmp(key, "visible") == 0)
			json_object_set_new(paste, "visible", json_boolean(strcmp(val, "on") == 0));
		else if (strcmp(key, "raw") == 0)
			raw = strcmp(val, "on") == 0;
	}

	if (database_insert(paste) < 0)
		page_status(req, KHTTP_500);
	else {
		id = ju_get_string(paste, "id");
		scheme = req->scheme == KSCHEME_HTTP ? "http" : "https";

		if (raw) {
			/* For CLI users (e.g. paster) just print the location. */
			khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_201]);
			khttp_body(req);
			khttp_printf(req, "%s://%s/paste/%s\n", scheme, req->host, id);
		} else {
			/* Otherwise, redirect to paste details. */
			khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
			khttp_head(req, kresps[KRESP_LOCATION], "/paste/%s", id);
			khttp_body(req);
		}

		khttp_free(req);
	}

	json_decref(paste);
}

#include "log.h"

void
page_new_render(struct kreq *req, json_t *paste)
{
	assert(req);

	page(req, KHTTP_200, html_new, ju_extend(paste, "{ss so so}",
		"pagetitle", TITLE,
		"durations", ju_durations(),
		"languages", ju_languages(ju_get_string(paste, "language"))
	));
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
