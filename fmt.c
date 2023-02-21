/*
 * fmt.c -- page formatter
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
#include <err.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "fmt.h"
#include "util.h"

struct tpl {
	struct kreq *req;
	struct khtmlreq html;
	const struct fmt_printer *printers;
	const void *data;
};

static int
callback(size_t index, void *arg)
{
	struct tpl *tpl = arg;

	tpl->printers[index].printer(tpl->req, &tpl->html, tpl->data);

	return 1;
}

static void
kt_init(struct ktemplate *kt, struct tpl *tpl)
{
	/*
	 * We need to create a list of strings to be set in kt->key from the
	 * printer list.
	 */
	size_t count = 0;
	const char **list;

	while (tpl->printers[count].keyword)
		count++;

	list = ecalloc(count, sizeof (char *));

	for (size_t i = 0; i < count; ++i)
		list[i] = tpl->printers[i].keyword;

	kt->key = list;
	kt->keysz = count;
	kt->arg = tpl;
	kt->cb = callback;

	/* HTML printer. */
	if (khtml_open(&tpl->html, tpl->req, 0) != KCGI_OK)
		errx(1, "khtml_open");
}

static inline void
kt_free(struct ktemplate *kt, struct tpl *tpl)
{
	free((const char **)kt->key);
	khtml_close(&tpl->html);
}

void
fmt(struct kreq *req,
    const unsigned char *html,
    const void *data,
    const struct fmt_printer *printers)
{
	assert(req);
	assert(html);

	const char *str = (const char *)html;
	struct ktemplate kt;
	struct tpl tpl = {
		.req = req,
		.printers = printers,
		.data = data
	};

	if (printers) {
		kt_init(&kt, &tpl);
		khttp_template_buf(req, &kt, str, strlen(str));
		kt_free(&kt, &tpl);
	} else
		khttp_template_buf(req, NULL, str, strlen(str));
}
