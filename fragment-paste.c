/*
 * fragment-paste.c -- paste index renderer
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
#include <time.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "fragment-paste.h"
#include "fragment.h"
#include "paste.h"
#include "util.h"

struct template {
	struct kreq *req;
	const struct paste *paste;
};

static const char *keywords[] = {
	"author",
	"date",
	"expiration",
	"id",
	"language",
	"title"
};

static int
template(size_t keyword, void *arg)
{
	struct template *tp = arg;
	struct khtmlreq html;

	khtml_open(&html, tp->req, KHTML_PRETTY);

	switch (keyword) {
	case 0:
		khtml_puts(&html, tp->paste->author);
		break;
	case 1:
		khtml_puts(&html, bstrftime("%c", localtime(&tp->paste->timestamp)));
		break;
	case 2:
		khtml_puts(&html, ttl(tp->paste->timestamp, tp->paste->duration));
		break;
	case 3:
		khtml_puts(&html, tp->paste->id);
		break;
	case 4:
		khtml_puts(&html, tp->paste->language);
		break;
	case 5:
		khtml_puts(&html, tp->paste->title);
		break;
	default:
		break;
	}

	khtml_close(&html);

	return 1;
}

void
fragment_paste(struct kreq *r, const struct paste *paste)
{
	assert(r);
	assert(paste);

	struct template data = {
		.req = r,
		.paste = paste,
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.cb = template,
		.arg = &data
	};

	fragment(r, &kt, "fragments/paste.html");
}
