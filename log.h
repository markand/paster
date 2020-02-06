/*
 * log.h -- logging routines
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

#ifndef PASTER_LOG_H
#define PASTER_LOG_H

#include <stdarg.h>

enum log_level {
	LOG_LEVEL_WARNING = 1,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG
};

#define log_debug(...)  log_write(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_warn(...)   log_write(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_info(...)   log_write(LOG_LEVEL_INFO, __VA_ARGS__)

void
log_open(void);

void
log_write(enum log_level level, const char *fmt, ...);

void
log_vwrite(enum log_level level, const char *fmt, va_list ap);

void
log_finish(void);

#endif /* !PASTER_LOG_H */
