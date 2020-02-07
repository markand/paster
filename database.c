/*
 * database.c -- sqlite storage
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

#include "database.h"
#include "log.h"
#include "paste.h"
#include "util.h"

static sqlite3 *db;

static const char *sql_init =
	"BEGIN EXCLUSIVE TRANSACTION;\n"
	"\n"
	"CREATE TABLE IF NOT EXISTS paste(\n"
	"  uuid TEXT PRIMARY KEY,\n"
	"  title TEXT,\n"
	"  author TEXT,\n"
	"  language TEXT,\n"
	"  code TEXT,\n"
	"  date INT DEFAULT CURRENT_TIMESTAMP,\n"
	"  visible INTEGER DEFAULT 1,\n"
	"  duration INT\n"
	");\n"
	"\n"
	"END TRANSACTION";

static const char *sql_get =
	"SELECT uuid\n"
	"     , title\n"
	"     , author\n"
	"     , language\n"
	"     , code\n"
	"     , strftime('%s', date)\n"
	"     , visible\n"
	"     , duration\n"
	"  FROM paste\n"
	" WHERE uuid = ?";

static const char *sql_insert =
	"INSERT INTO paste(\n"
	"  uuid,\n"
	"  title,\n"
	"  author,\n"
	"  language,\n"
	"  code,\n"
	"  visible,\n"
	"  duration\n"
	") VALUES (?, ?, ?, ?, ?, ?, ?)";

static const char *sql_recents =
	"SELECT uuid\n"
	"     , title\n"
	"     , author\n"
	"     , language\n"
	"     , code\n"
	"     , strftime('%s', date) AS date\n"
	"     , visible\n"
	"     , duration\n"
	"  FROM paste\n"
	" WHERE visible = 1\n"
	" ORDER BY date DESC\n"
	" LIMIT ?\n";

static const char *sql_clear =
	"BEGIN EXCLUSIVE TRANSACTION;\n"
	"\n"
	"DELETE\n"
	"  FROM paste\n"
	" WHERE strftime('%s', 'now') - strftime('%s', date) >= duration;"
	"\n"
	"END TRANSACTION";

static const char *sql_search =
	"SELECT uuid\n"
	"     , title\n"
	"     , author\n"
	"     , language\n"
	"     , code\n"
	"     , strftime('%s', date) AS date\n"
	"     , visible\n"
	"     , duration\n"
	"  FROM paste\n"
	" WHERE title like ?\n"
	"   AND author like ?\n"
	"   AND language like ?\n"
	"   AND visible = 1\n"
	" ORDER BY date DESC\n"
	" LIMIT ?\n";

/* sqlite3 use const unsigned char *. */
static char *
dup(const unsigned char *s)
{
	return estrdup(s ? (const char *)(s) : "");
}

static const char *
create_id(void)
{
	static char uuid[256];

	/*
	 * Not a very strong generation but does not require to link against
	 * util-linux.
	 *
	 * See https://stackoverflow.com/questions/2174768/generating-random-uuids-in-linux
	 */
	sprintf(uuid, "%x%x-%x-%x-%x-%x%x%x",
	    rand(), rand(),
	    rand(),
	    ((rand() & 0x0fff) | 0x4000),
	    rand() % 0x3fff + 0x8000,
	    rand(), rand(), rand());

	return uuid;
}

static void
convert(sqlite3_stmt *stmt, struct paste *paste)
{
	paste->uuid = dup(sqlite3_column_text(stmt, 0));
	paste->title = dup(sqlite3_column_text(stmt, 1));
	paste->author = dup(sqlite3_column_text(stmt, 2));
	paste->language = dup(sqlite3_column_text(stmt, 3));
	paste->code = dup(sqlite3_column_text(stmt, 4));
	paste->timestamp = sqlite3_column_int64(stmt, 5);
	paste->visible = sqlite3_column_int(stmt, 6);
	paste->duration = sqlite3_column_int64(stmt, 7);
}

bool
database_open(const char *path)
{
	assert(path);

	log_info("database: opening %s", path);

	if (sqlite3_open(path, &db) != SQLITE_OK) {
		log_warn("database: unable to open %s: %s", path, sqlite3_errmsg(db));
		return false;
	}

	if (sqlite3_exec(db, sql_init, NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: unable to initialize %s: %s", path, sqlite3_errmsg(db));
		return false;
	}

	return true;
}

bool
database_recents(struct paste *pastes, size_t *max)
{
	assert(pastes);
	assert(max);

	sqlite3_stmt *stmt = NULL;

	memset(pastes, 0, *max * sizeof (struct paste));
	log_debug("database: accessing most recents");

	if (sqlite3_prepare(db, sql_recents, -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_int64(stmt, 1, *max) != SQLITE_OK)
		goto sqlite_err;

	size_t i = 0;

	for (; i < *max && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		convert(stmt, &pastes[i]);

	log_debug("database: found %zu pastes", i);
	sqlite3_finalize(stmt);
	*max = i;

	return true;

sqlite_err:
	log_warn("database: error (recents): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	return (*max = 0);
}

bool
database_get(struct paste *paste, const char *uuid)
{
	assert(paste);
	assert(uuid);

	memset(paste, 0, sizeof (struct paste));
	log_debug("database: accessing paste with uuid: %s", uuid);

	sqlite3_stmt* stmt = NULL;

	if (sqlite3_prepare(db, sql_get, -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_text(stmt, 1, uuid, -1, NULL) != SQLITE_OK)
		goto sqlite_err;

	switch (sqlite3_step(stmt)) {
	case SQLITE_ROW:
		convert(stmt, paste);
		break;
	case SQLITE_MISUSE:
	case SQLITE_ERROR:
		goto sqlite_err;
	default:
		break;
	}

	sqlite3_finalize(stmt);

	return true;

sqlite_err:
	if (stmt)
		sqlite3_finalize(stmt);

	log_warn("database: error (get): %s", sqlite3_errmsg(db));

	return false;
}

bool
database_insert(struct paste *paste)
{
	assert(paste);

	sqlite3_stmt* stmt = NULL;
	log_debug("database: creating new paste");

	if (sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: could not lock database: %s", sqlite3_errmsg(db));
		return false;
	}

	if (sqlite3_prepare(db, sql_insert, -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;

	/* Create a new uuid first. */
	paste->uuid = estrdup(create_id());

	sqlite3_bind_text(stmt, 1, paste->uuid, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, paste->title, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, paste->author, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, paste->language, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 5, paste->code, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, paste->visible);
	sqlite3_bind_int64(stmt, 7, paste->duration);

	if (sqlite3_step(stmt) != SQLITE_DONE)
		goto sqlite_err;

	sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
	sqlite3_finalize(stmt);

	log_info("database: new paste (%s) from %s expires in one %lld seconds",
	    paste->uuid, paste->author, paste->duration);

	return true;

sqlite_err:
	log_warn("database: error (insert): %s", sqlite3_errmsg(db));
	sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);

	if (stmt)
		sqlite3_finalize(stmt);

	free(paste->uuid);
	paste->uuid = NULL;

	return false;
}

bool
database_search(struct paste *pastes,
                size_t *max,
                const char *title,
                const char *author,
                const char *language)
{
	assert(pastes);
	assert(max);

	sqlite3_stmt *stmt = NULL;

	log_debug("database: searching title=%s, author=%s, language=%s",
	    title    ? title    : "",
	    author   ? author   : "",
	    language ? language : "");

	memset(pastes, 0, *max * sizeof (struct paste));

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
	if (sqlite3_bind_int64(stmt, 4, *max) != SQLITE_OK)
		goto sqlite_err;

	size_t i = 0;

	for (; i < *max && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		convert(stmt, &pastes[i]);

	log_debug("database: found %zu pastes", i);
	sqlite3_finalize(stmt);
	*max = i;

	return true;

sqlite_err:
	log_warn("database: error (search): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	return (*max = 0);
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
