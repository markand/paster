/*
 * fmt-paste.c -- page formatter for pastes
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

#include <kcgi.h>
#include <kcgihtml.h>

#include "fmt-paste.h"
#include "fmt.h"
#include "paste.h"
#include "util.h"

#include "html/paste.h"
#include "html/paste-table.h"

static void
print_id(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)html;

	khttp_puts(req, ((const struct paste *)data)->id);
}

static void
print_title(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)req;

	khtml_puts(html, ((const struct paste *)data)->title);
}

static void
print_author(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)req;

	khtml_puts(html, ((const struct paste *)data)->author);
}

static void
print_language(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)html;

	khttp_puts(req, ((const struct paste *)data)->language);
}

static void
print_date(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)html;

	const struct paste *paste = data;

	khttp_puts(req, bstrftime("%c", localtime(&paste->timestamp)));
}

static void
print_expiration(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)html;

	const struct paste *paste = data;

	khttp_puts(req, ttl(paste->timestamp, paste->duration));
}

#include <stdio.h>
static void
print_pastes(struct kreq *req, struct khtmlreq *html, const void *data)
{
	(void)html;

	fmt_pastes(req, data);
}

/*
 * Generate each column for the given paste.
 */
void
fmt_paste(struct kreq *req, const struct paste *paste)
{
	assert(req);
	assert(paste);

	fmt(req, html_paste, paste, (const struct fmt_printer []) {
		{ "id",         print_id                },
		{ "title",      print_title             },
		{ "author",     print_author            },
		{ "language",   print_language          },
		{ "date",       print_date              },
		{ "expiration", print_expiration        },
		{ NULL,         NULL                    }
	});
}

/*
 * Generate each row from all pastes
 */
void
fmt_pastes(struct kreq *req, const struct fmt_paste_vec *vec)
{
	for (size_t i = 0; i < vec->pastesz; ++i)
		fmt_paste(req, &vec->pastes[i]);
}

/*
 * Generate an HTML table with all pastes inside as long as there is a
 * keyword.
 */
void
fmt_paste_table(struct kreq *req, const struct fmt_paste_vec *vec)
{
	assert(req);
	assert(pastes);

	fmt(req, html_paste_table, vec, (const struct fmt_printer []) {
		{ "pastes",     print_pastes    },
		{ NULL,         NULL            }
	});
}
