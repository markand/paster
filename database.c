/*
 * database.c -- sqlite storage
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

#include "database.h"
#include "json-util.h"
#include "log.h"
#include "util.h"

#include "sql/clear.h"
#include "sql/get.h"
#include "sql/init.h"
#include "sql/insert.h"
#include "sql/recents.h"
#include "sql/search.h"

#define ID_MAX (12 + 1)

static sqlite3 *db;

static inline json_t *
convert(sqlite3_stmt *stmt)
{
	return json_pack("{ss ss ss ss ss sI si si}",
		"id",           sqlite3_column_text(stmt, 0),
		"title",        sqlite3_column_text(stmt, 1),
		"author",       sqlite3_column_text(stmt, 2),
		"language",     sqlite3_column_text(stmt, 3),
		"code",         sqlite3_column_text(stmt, 4),
		"timestamp",    (json_int_t)sqlite3_column_int64(stmt, 5),
		"visible",      sqlite3_column_int(stmt, 6),
		"duration",     sqlite3_column_int(stmt, 7)
	);
}

static int
exists(const char *id)
{
	assert(id);

	sqlite3_stmt *stmt = NULL;
	int ret = 1;

	if (sqlite3_prepare(db, sql_get, -1, &stmt, NULL) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, id, -1, NULL);
		ret = sqlite3_step(stmt) == SQLITE_ROW;
		sqlite3_finalize(stmt);
	}

	return ret;
}

static const char *
create_id(char *id)
{
	static const char table[] = "abcdefghijklmnopqrstuvwxyz1234567890";

	for (int i = 0; i < ID_MAX; ++i)
		id[i] = table[rand() % (sizeof (table) - 1)];

	id[ID_MAX - 1] = 0;
}

static int
set_id(json_t *paste)
{
	/*
	 * Avoid infinite loop, we only try to create a new id in 30 steps.
	 *
	 * On error, the function `exist` returns true to indicate we should
	 * not try to save with that id.
	 */
	int tries = 0;
	char id[ID_MAX];

	do {
		create_id(id);
	} while (++tries < 30 && exists(id));

	if (tries >= 30)
		return -1;

	json_object_set_new(paste, "id", json_string(id));

	return 0;
}

int
database_open(const char *path)
{
	assert(path);

	log_info("database: opening %s", path);

	if (sqlite3_open(path, &db) != SQLITE_OK) {
		log_warn("database: unable to open %s: %s", path, sqlite3_errmsg(db));
		return -1;
	}

	/* Wait for 30 seconds to lock the database. */
	sqlite3_busy_timeout(db, 30000);

	if (sqlite3_exec(db, sql_init, NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: unable to initialize %s: %s", path, sqlite3_errmsg(db));
		return -1;
	}

	return 0;
}

json_t *
database_recents(size_t limit)
{
	json_t *array = NULL;
	sqlite3_stmt *stmt = NULL;
	size_t i = 0;

	log_debug("database: accessing most recents");

	if (sqlite3_prepare(db, sql_recents, -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_int64(stmt, 1, limit) != SQLITE_OK)
		goto sqlite_err;

	array = json_array();

	for (; i < limit && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		json_array_append_new(array, convert(stmt));

	log_debug("database: found %zu pastes", i);
	sqlite3_finalize(stmt);

	return array;

sqlite_err:
	log_warn("database: error (recents): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	return NULL;
}

json_t *
database_get(const char *id)
{
	assert(id);

	json_t *object = NULL;
	sqlite3_stmt* stmt = NULL;

	log_debug("database: accessing paste with uuid: %s", id);

	if (sqlite3_prepare(db, sql_get, -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_text(stmt, 1, id, -1, NULL) != SQLITE_OK)
		goto sqlite_err;

	switch (sqlite3_step(stmt)) {
	case SQLITE_ROW:
		object = convert(stmt);
		break;
	case SQLITE_MISUSE:
	case SQLITE_ERROR:
		goto sqlite_err;
	default:
		break;
	}

	sqlite3_finalize(stmt);

	return object;

sqlite_err:
	if (stmt)
		sqlite3_finalize(stmt);

	log_warn("database: error (get): %s", sqlite3_errmsg(db));

	return NULL;
}

int
database_insert(json_t *paste)
{
	assert(paste);

	sqlite3_stmt *stmt = NULL;

	log_debug("database: creating new paste");

	if (sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: could not lock database: %s", sqlite3_errmsg(db));
		return -1;
	}

	if (set_id(paste) < 0) {
		log_warn("database: unable to randomize unique identifier");
		sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
		return -1;
	}

	if (sqlite3_prepare(db, sql_insert, -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;

	sqlite3_bind_text(stmt, 1, ju_get_string(paste, "id"), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, ju_get_string(paste, "title"), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, ju_get_string(paste, "author"), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, ju_get_string(paste, "language"), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 5, ju_get_string(paste, "code"), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, ju_get_bool(paste, "visible"));
	sqlite3_bind_int64(stmt, 7, ju_get_int(paste, "duration"));

	if (sqlite3_step(stmt) != SQLITE_DONE)
		goto sqlite_err;

	sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
	sqlite3_finalize(stmt);

	log_info("database: new paste (%s) from %s expires in one %lld seconds",
	    ju_get_string(paste, "id"),
	    ju_get_string(paste, "author"),
	    ju_get_int(paste, "duration")
	);

	return 0;

sqlite_err:
	log_warn("database: error (insert): %s", sqlite3_errmsg(db));
	sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);

	if (stmt)
		sqlite3_finalize(stmt);

	/* Make sure it is not used anymore. */
	json_object_del(paste, "id");

	return 0;
}

json_t *
database_search(size_t limit,
                const char *title,
                const char *author,
                const char *language)
{
	json_t *array = NULL;
	sqlite3_stmt *stmt = NULL;
	size_t i = 0;

	log_debug("database: searching title=%s, author=%s, language=%s",
	    title    ? title    : "",
	    author   ? author   : "",
	    language ? language : "");

	/* Select everything if not specified. */
	title    = title    ? title    : "%";
	author   = author   ? author   : "%";
	language = language ? language : "%";

	if (sqlite3_prepare(db, sql_search, -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 2, author, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 3, language, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_int64(stmt, 4, limit) != SQLITE_OK)
		goto sqlite_err;

	array = json_array();

	for (; i < limit && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		json_array_append_new(array, convert(stmt));

	log_debug("database: found %zu pastes", i);
	sqlite3_finalize(stmt);

	return array;

sqlite_err:
	log_warn("database: error (search): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	return NULL;
}

void
database_clear(void)
{
	log_debug("database: clearing deprecated pastes");

	if (sqlite3_exec(db, sql_clear, NULL, NULL, NULL) != SQLITE_OK)
		log_warn("database: error (clear): %s\n", sqlite3_errmsg(db));
}

void
database_finish(void)
{
	log_debug("database: closing");

	if (db) {
		sqlite3_close(db);
		db = NULL;
	}
}
