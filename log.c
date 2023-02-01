/*
 * log.c -- logging routines
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
#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "log.h"

static int syslog_levels[] = {
	[LOG_LEVEL_DEBUG]       = LOG_DEBUG,
	[LOG_LEVEL_INFO]        = LOG_INFO,
	[LOG_LEVEL_WARNING]     = LOG_WARNING
};

void
log_open(void)
{
	if (config.verbosity > 0)
		openlog("paster", 0, LOG_USER);
}

void
log_write(enum log_level level, const char *fmt, ...)
{
	assert(level >= LOG_LEVEL_WARNING && level <= LOG_LEVEL_DEBUG);
	assert(fmt);

	if (config.verbosity >= level) {
		va_list ap;

		va_start(ap, fmt);
		log_vwrite(level, fmt, ap);
		va_end(ap);
	}
}

void
log_vwrite(enum log_level level, const char *fmt, va_list ap)
{
	assert(level >= LOG_LEVEL_WARNING && level <= LOG_LEVEL_DEBUG);
	assert(fmt);

	char line[BUFSIZ];

	vsnprintf(line, sizeof (line), fmt, ap);
	syslog(syslog_levels[level], "%s", line);
}

void
log_finish(void)
{
	if (config.verbosity > 0)
		closelog();
}
