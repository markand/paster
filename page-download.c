/*
 * page-download.c -- page /download/<id>
 *
 * Copyright (c) 2020-2021 David Demelier <markand@malikania.fr>
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
#include "page.h"
#include "paste.h"

static void
get(struct kreq *r)
{
	struct paste paste = {0};

	if (!database_get(&paste, r->path))
		page(r, NULL, KHTTP_404, "404.html");
	else {
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
#if 0
		/* TODO: this seems to generated truncated files. */
		khttp_head(r, kresps[KRESP_CONTENT_LENGTH], "%zu", strlen(paste.code));
#endif
		khttp_head(r, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(r, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s.%s\"", paste.id, paste.language);
		khttp_body(r);
		khttp_puts(r, paste.code);
		khttp_free(r);
		paste_finish(&paste);
	}
}

void
page_download(struct kreq *r)
{
	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	default:
		page(r, NULL, KHTTP_400, "400.html");
		break;
	}
}
