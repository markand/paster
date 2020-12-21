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

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "database.h"
#include "http.h"
#include "log.h"
#include "util.h"

static void
defaults(void)
{
	snprintf(config.databasepath, sizeof (config.databasepath),
	    "%s", VARDIR "/paster/paster.db");
}

static void
init(void)
{
	srand(time(NULL));
	log_open();

	if (!config.databasepath[0])
		die("abort: no database specified\n");
	if (!database_open(config.databasepath))
		die("abort: could not open database\n");
}

static void
quit(void)
{
	database_finish();
	log_finish();
}

static noreturn void
usage(void)
{
	fprintf(stderr, "usage: paster [-fqv] [-d database-path] [-t theme-directory]\n");
	exit(1);
}
 
int
main(int argc, char **argv, char **env)
{
	const char *value;
	int opt;
	void (*run)(void) = &(http_cgi_run);

	defaults();

	/* Seek environment variables before options. */
	if ((value = getenv("PASTERD_DATABASE_PATH")))
		snprintf(config.databasepath, sizeof (config.databasepath), "%s", value);
	if ((value = getenv("PASTERD_THEME_DIR")))
		snprintf(config.themedir, sizeof (config.themedir), "%s", value);
	if ((value = getenv("PASTERD_VERBOSITY")))
		config.verbosity = atoi(value);

	while ((opt = getopt(argc, argv, "d:ft:qv")) != -1) {
		switch (opt) {
		case 'd':
			snprintf(config.databasepath, sizeof (config.databasepath), "%s", optarg);
			break;
		case 't':
			snprintf(config.themedir, sizeof (config.themedir), "%s", optarg);
			break;
		case 'f':
			run = &(http_fcgi_run);
			break;
		case 'v':
			config.verbosity++;
			break;
		case 'q':
			config.verbosity = 0;
			break;
		default:
			usage();
			break;
		}
	}

	init();
	run();
	quit();
}
