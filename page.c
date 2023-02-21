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

#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "fmt.h"
#include "page.h"
#include "util.h"

#include "html/header.h"
#include "html/footer.h"

static void
print_title(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)req;

	khtml_printf(html, "%s", (const char *)data);
}

void
page(struct kreq *req, const struct ktemplate *tmpl, enum khttp status, const char *file, const char *title)
{
	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_body(req);

	fmt(req, html_header, title, (const struct fmt_printer []) {
		{ "title",      print_title     },
		{ NULL,         NULL            }
	});
	fmt(req, html_footer, NULL, NULL);
	khttp_free(req);
}

void
page2(struct kreq *req,
      enum khttp status,
      const char *title,
      const unsigned char *html,
      const void *data,
      const struct fmt_printer *printers)
{
	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_body(req);

	fmt(req, html_header, title, (const struct fmt_printer []) {
		{ "title",      print_title     },
		{ NULL,         NULL            }
	});
	fmt(req, html, data, printers);
	fmt(req, html_footer, NULL, NULL);
	khttp_free(req);
}
