/*
 * page.c -- page renderer
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

#include <mustach-jansson.h>

#include "config.h"
#include "page.h"
#include "util.h"

#include "html/footer.h"
#include "html/header.h"
#include "html/status.h"

static const int statustab[] = {
	[KHTTP_200] = 200,
	[KHTTP_400] = 400,
	[KHTTP_404] = 404,
	[KHTTP_500] = 500
};

static const char * const statusmsg[] = {
	[KHTTP_200] = "OK",
	[KHTTP_400] = "Bad Request",
	[KHTTP_404] = "Not Found",
	[KHTTP_500] = "Internal Server Error"
};

static int
writer(void *data, const char *buffer, size_t size)
{
	struct kreq *req = data;

	khttp_write(req, buffer, size);

	return MUSTACH_OK;
}

static void
format(struct kreq *req, const char *html, json_t *doc)
{
	if (!doc)
		khttp_template_buf(req, NULL, html, strlen(html));
	else
		mustach_jansson_write(html, strlen(html), doc, 0, writer, req);
}

void
page(struct kreq *req, enum khttp status, const unsigned char *html, json_t *doc)
{
	assert(req);
	assert(html);

	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_body(req);
	format(req, (const char *)html_header, doc);
	format(req, (const char *)html, doc);
	format(req, (const char *)html_footer, doc);
	khttp_free(req);

	if (doc)
		json_decref(doc);
}

void
page_status(struct kreq *req, enum khttp status)
{
	assert(req);

	page(req, status, html_status, json_pack("{si ss}",
		"code",         statustab[status],
		"status",       statusmsg[status]
	));
}
