/*
 * page-static.c -- page /static
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
#include <sys/stat.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "page-status.h"
#include "page.h"

static inline enum kmime
mimetype(const struct kreq *req)
{
	switch (req->mime) {
	case KMIME_TEXT_HTML:
	case KMIME_TEXT_CSS:
		return req->mime;
	default:
		return KMIME_APP_OCTET_STREAM;
	}
}

static void
get(struct kreq *req)
{
	struct stat st;
	char path[PATH_MAX];

	snprintf(path, sizeof (path), "%s%s", config.themedir, req->fullpath);

	if (stat(path, &st) < 0)
		page_status(req, KHTTP_404);
	else {
		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[mimetype(req)]);
		khttp_head(req, kresps[KRESP_CONTENT_LENGTH],
		    "%llu", (unsigned long long)(st.st_size));
		khttp_body(req);
		khttp_template(req, NULL, path);
		khttp_free(req);
	}
}

void
page_static(struct kreq *req)
{
	assert(req);

	switch (req->method) {
	case KMETHOD_GET:
		get(req);
		break;
	default:
		page_status(req, KHTTP_400);
		break;
	}
}
