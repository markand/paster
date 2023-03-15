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
#include "paste.h"
#include "page-new.h"
#include "page.h"
#include "util.h"

#include "html/new.h"

static const struct {
	const char *title;
	long long int secs;
} durations[] = {
	{ "day",        PASTE_DURATION_DAY      },
	{ "hour",       PASTE_DURATION_HOUR     },
	{ "week",       PASTE_DURATION_WEEK     },
	{ "month",      PASTE_DURATION_MONTH    },
};

static const struct paste paste_default = {
	.id = "",
	.title = "unknown",
	.author = "anonymous",
	.language = "nohighlight",
	.code = ""
};

static long long int
duration(const char *val)
{
	for (size_t i = 0; i < NELEM(durations); ++i)
		if (strcmp(val, durations[i].title) == 0)
			return durations[i].secs;

	/* Default to month. */
	return PASTE_DURATION_MONTH;
}

static void
get(struct kreq *r)
{
	page_new_render(r, NULL);
}

static void
post(struct kreq *r)
{
	struct paste paste = {
		.author         = estrdup("Anonymous"),
		.title          = estrdup("Untitled"),
		.language       = estrdup("nohighlight"),
		.code           = estrdup(""),
		.visible        = true,
		.duration       = PASTE_DURATION_DAY
	};
	int raw = 0;

	for (size_t i = 0; i < r->fieldsz; ++i) {
		const char *key = r->fields[i].key;
		const char *val = r->fields[i].val;

		if (strcmp(key, "title") == 0)
			replace(&paste.title, val);
		else if (strcmp(key, "author") == 0)
			replace(&paste.author, val);
		else if (strcmp(key, "language") == 0)
			replace(&paste.language, val);
		else if (strcmp(key, "duration") == 0)
			paste.duration = duration(val);
		else if (strcmp(key, "code") == 0)
			paste.code = estrdup(val);
		else if (strcmp(key, "private") == 0)
			paste.visible = strcmp(val, "on") != 0;
		else if (strcmp(key, "raw") == 0) {
			raw = strcmp(val, "on") == 0;
		}
	}

	if (!database_insert(&paste))
		page_status(r,KHTTP_500);
	else {
		if (raw) {
			/* For CLI users (e.g. paster) just print the location. */
			khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_201]);
			khttp_body(r);
			khttp_printf(r, "%s://%s/paste/%s\n",
			    r->scheme == KSCHEME_HTTP ? "http" : "https",
			    r->host, paste.id);
			khttp_free(r);
		} else {
			/* Otherwise, redirect to paste details. */
			khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
			khttp_head(r, kresps[KRESP_LOCATION], "/paste/%s", paste.id);
			khttp_body(r);
			khttp_free(r);
		}
	}

	paste_finish(&paste);
}

static json_t *
create_languages(const struct paste *paste)
{
	json_t *array, *obj;

	array = json_array();

	for (size_t i = 0; i < languagesz; ++i) {
		if (strcmp(languages[i], paste->language) == 0)
			obj = json_pack("{ss ss}",
				"name",         languages[i],
				"selected",     "selected"
			);
		else
			obj = json_pack("{ss}", "name", languages[i]);

		json_array_append_new(array, obj);
	}

	return array;
}

static inline json_t *
create_durations(void)
{
	json_t *array = json_array();

	for (size_t i = 0; i < NELEM(durations); ++i)
		json_array_append_new(array, json_pack("{ss}", "value", durations[i].title));

	return array;
}

void
page_new_render(struct kreq *req, const struct paste *paste)
{
	assert(req);

	if (!paste)
		paste = &paste_default;

	page(req, KHTTP_200, html_new, json_pack("{ss ss so so ss}",
		"pagetitle",    "paster -- create new paste",
		"title",        paste->title,
		"languages",    create_languages(paste),
		"durations",    create_durations(),
		"code",         paste->code
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
