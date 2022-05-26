/*
 * page-new.c -- page /new
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
#include <string.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "database.h"
#include "fragment-duration.h"
#include "fragment-language.h"
#include "paste.h"
#include "page-new.h"
#include "page.h"
#include "util.h"

struct template {
	struct kreq *req;
	const struct paste *paste;
};

static const char *keywords[] = {
	"code",
	"durations",
	"languages",
	"title"
};

static const struct {
	const char *title;
	long long int secs;
} durations[] = {
	{ "day",        PASTE_DURATION_DAY      },
	{ "hour",       PASTE_DURATION_HOUR     },
	{ "week",       PASTE_DURATION_WEEK     },
	{ "month",      PASTE_DURATION_MONTH    },
};

static bool
is_selected(const struct paste *paste, const char *language)
{
	return paste && strcmp(paste->language, language) == 0;
}

static int
template(size_t keyword, void *arg)
{
	struct template *tp = arg;
	struct khtmlreq html;

	khtml_open(&html, tp->req, KHTML_PRETTY);

	switch (keyword) {
	case 0:
		if (tp->paste)
			khtml_puts(&html, tp->paste->code);
		break;
	case 1:
		for (size_t i = 0; i < NELEM(durations); ++i)
			fragment_duration(tp->req, durations[i].title);
		break;
	case 2:
		for (size_t i = 0; i < languagesz; ++i)
			fragment_language(tp->req, languages[i],
			    is_selected(tp->paste, languages[i]));
		break;
	case 3:
		if (tp->paste)
			khtml_puts(&html, tp->paste->title);
		break;
	default:
		break;
	}

	khtml_close(&html);

	return 1;
}

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
		page(r, NULL, KHTTP_500, "500.html", "500");
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

void
page_new_render(struct kreq *r, const struct paste *paste)
{
	struct template tp = {
		.req = r,
		.paste = paste
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.cb = template,
		.arg = &tp
	};

	page(r, &kt, KHTTP_200, "pages/new.html",
	    paste ? paste->title : "Create new paste");
}

void
page_new(struct kreq *r)
{
	assert(r);

	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	case KMETHOD_POST:
		post(r);
		break;
	default:
		break;
	}
}
