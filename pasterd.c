/*
 * pasterd.c -- main pasterd(8) file
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

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "database.h"
#include "http.h"
#include "log.h"
#include "util.h"

/*
 * Interval in seconds for each cleanup time. Since a paste has one hour
 * duration as minimal let's cleanup every hour.
 */
#define CLEANUP_INTERVAL 3600

static pthread_t thread;
static sig_atomic_t running = 1;

/*
 * This function runs in a thread, we open our own local database to let the
 * engine locks by itself.
 */
static void *
cleanup(void *data)
{
	(void)data;

	struct database db;
	sigset_t sigs;

	sigemptyset(&sigs);
	sigfillset(&sigs);
	pthread_sigmask(SIG_BLOCK, &sigs, NULL);

	for (;;) {
		sleep(CLEANUP_INTERVAL);

		if (database_open(&db, config.databasepath) == 0) {
			database_clear(&db);
			database_finish(&db);
		}
	}

	return NULL;
}

static void
stop(int n)
{
	running = 0;
}

static void
defaults(void)
{
	snprintf(config.databasepath, sizeof (config.databasepath),
	    "%s", VARDIR "/paster/paster.db");
}

static void
init(void)
{
	struct itimerspec spec = {
		.it_value = { .tv_sec = CLEANUP_INTERVAL },
		.it_interval = { .tv_sec = CLEANUP_INTERVAL }
	};
	struct sigaction sa = {0};

	/* Setup signal handlers. */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = stop;

	if (sigaction(SIGINT, &sa, NULL) < 0 || sigaction(SIGTERM, &sa, NULL) < 0)
		die("abort: sigaction: %s\n", strerror(errno));
	if (pthread_create(&thread, NULL, cleanup, NULL) < 0)
		die("abort: pthread_create: %s", strerror(errno));

	srand(time(NULL));
	log_open();

	if (!config.databasepath[0])
		die("abort: no database specified\n");
	if (database_open(&database, config.databasepath) < 0)
		die("abort: could not open database\n");
}

static void
run(void)
{
	while (running)
		http_fcgi_run();
}

static void
finish(void)
{
	database_finish(&database);
	log_finish();
}

static void
usage(void)
{
	fprintf(stderr, "usage: paster [-qv] [-d database-path] [-t theme-directory]\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	const char *value;
	int opt;

	defaults();

	/* Seek environment variables before options. */
	if ((value = getenv("PASTERD_DATABASE_PATH")))
		snprintf(config.databasepath, sizeof (config.databasepath), "%s", value);
	if ((value = getenv("PASTERD_THEME_DIR")))
		snprintf(config.themedir, sizeof (config.themedir), "%s", value);
	if ((value = getenv("PASTERD_VERBOSITY")))
		config.verbosity = atoi(value);

	while ((opt = getopt(argc, argv, "d:t:qv")) != -1) {
		switch (opt) {
		case 'd':
			snprintf(config.databasepath, sizeof (config.databasepath), "%s", optarg);
			break;
		case 't':
			snprintf(config.themedir, sizeof (config.themedir), "%s", optarg);
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
	finish();
}
