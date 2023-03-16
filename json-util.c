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
ju_date(time_t timestamp)
{
	return json_string(bstrftime("%c", localtime(&timestamp)));
}

json_t *
ju_expires(time_t timestamp, int duration)
{
	return json_string(ttl(timestamp, duration));
}

const char *
ju_get_string(const json_t *doc, const char *key)
{
	const json_t *val;

	if (!doc || !(val = json_object_get(doc, key)) || !json_is_string(val))
		return NULL;

	return json_string_value(val);
}

intmax_t
ju_get_int(const json_t *doc, const char *key)
{
	const json_t *val;

	if (!doc || !(val = json_object_get(doc, key)) || !json_is_integer(val))
		return 0;

	return json_integer_value(val);
}

int
ju_get_bool(const json_t *doc, const char *key)
{
	const json_t *val;

	if (!doc || !(val = json_object_get(doc, key)) || !json_is_boolean(val))
		return 0;

	return json_boolean_value(val);
}

json_t *
ju_paste_new(void)
{
	return json_pack("{ss ss ss ss si si}",
		"title",        "Untitled",
		"author",       "Anonymous",
		"language",     "nohighlight",
		"code",         "The best code is no code",
		"visible",      0,
		"duration",     PASTE_DURATION_HOUR
	);
}

json_t *
ju_extend(json_t *doc, const char *fmt, ...)
{
	assert(fmt);

	json_t *ret, *val;
	json_error_t err;
	va_list ap;
	const char *key;

	va_start(ap, fmt);
	ret = json_vpack_ex(&err, 0, fmt, ap);
	va_end(ap);

	/* Now steal every nodes from doc and put them in ret. */
	if (doc) {
		json_object_foreach(doc, key, val)
			json_object_set(ret, key, val);

		json_decref(doc);
	}

	return ret;
}
