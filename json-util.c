/*
 * json-util.c -- utilities for JSON
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

#include "json-util.h"
#include "util.h"
#include "paste.h"

json_t *
ju_languages(const char *selected)
{
	json_t *array, *obj;

	array = json_array();

	for (size_t i = 0; i < languagesz; ++i) {
		if (selected && strcmp(languages[i], selected) == 0)
			obj = json_pack("{ss ss}",
				"name",         languages[i],
				"selected",     "selected"
			);
		else
			obj = json_pack("{ss}", "name", languages[i]);

		json_array_append_new(array, obj);
	}

	return array;
}

json_t *
ju_durations(void)
{
	json_t *array = json_array();

	for (size_t i = 0; i < durationsz; ++i)
		json_array_append_new(array, json_pack("{ss}",
		    "value", durations[i].title)
		);

	return array;
}

json_t *
ju_date(const struct paste *paste)
{
	assert(paste);

	return json_string(bstrftime("%c", localtime(&paste->timestamp)));
}

json_t *
ju_expiration(const struct paste *paste)
{
	assert(paste);

	return json_string(ttl(paste->timestamp, paste->duration));
}
