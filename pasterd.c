/*
 * pasterd.c -- main pasterd(8) file
 *
 * Copyright (c) 2020 David Demelier <markand@malikania.fr>
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <kcgi.h>


#include <stdlib.h>
#include <time.h>

#include "database.h"

#if 0

static const char *pages[] = {
	"index",
	"new",
	"fork",
	"get"
};


static int
process(struct kreq *req)
{
	assert(req);

	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_PLAIN]);
	khttp_body(req);
	khttp_puts(req, "== begin ==\n");

	for (size_t i = 0; i < req->fieldsz; ++i) {
		khttp_puts(req, req->fields[i].key);
		khttp_putc(req, '=');
		khttp_puts(req, req->fields[i].val);
		khttp_putc(req, '\n');
	}

	khttp_puts(req, "== end ==\n");
	khttp_free(req);

	return 0;
}

static int
kcgi_run(void)
{
	struct kreq req;
	struct kfcgi *fcgi;

	if (khttp_fcgi_init(&fcgi, NULL, 0, pages, 4, 0) != KCGI_OK)
		return 1;
 
	while (khttp_fcgi_parse(fcgi, &req) == KCGI_OK)
		process(&req);
 
	khttp_fcgi_free(fcgi);

	return 0;
}

static int
cgi_run(void)
{
	struct kreq req;

	if (khttp_parse(&req, NULL, 0, pages, 4, 0) != KCGI_OK)
		return 1;

	return process(&req);
}

#endif

#include "util.h"
#include "paste.h"
#include "log.h"
 
int
main(int argc, char **argv)
{
	srand(time(NULL));

	(void)argc;
	(void)argv;

	struct paste paste = {
		.title = estrdup("Test de C++"),
		.author = estrdup("David Demelier"),
		.language = estrdup("C"),
		.code = estrdup("int main(void) { }"),
		.duration = 60
	};

	log_open();
	database_open("test.db");



	struct paste pastes[10];
	size_t n = 10;


	database_clear();
	database_recents(pastes, &n);
	for (size_t i = 0; i < n; ++i) {
		printf("%s, %s, %lld\n", pastes[i].uuid, pastes[i].author, (long long int)pastes[i].timestamp);
		paste_finish(&pastes[i]);
	}



	paste_finish(&paste);
	database_finish();
	log_finish();

#if 0
	int opt;
	int (*run)(void) = &(cgi_run);

	while ((opt = getopt(argc, argv, "f")) != -1) {
		switch (opt) {
		case 'f':
			run = &(kcgi_run);
			break;
		default:
			break;
		}
	}

	return run();
#endif
	
#if 0
	(void)argc;
	(void)argv;

#endif
}
