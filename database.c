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
#include "log.h"
#include "paste.h"
#include "util.h"

#include "sql/clear.h"
#include "sql/get.h"
#include "sql/init.h"
#include "sql/insert.h"
#include "sql/recents.h"
#include "sql/search.h"

#define CHAR(sql) (const char *)(sql)

static sqlite3 *db;

static char *
dup(const unsigned char *s)
{
	return estrdup(s ? (const char *)(s) : "");
}

static void
convert(sqlite3_stmt *stmt, struct paste *paste)
{
	paste->id = dup(sqlite3_column_text(stmt, 0));
	paste->title = dup(sqlite3_column_text(stmt, 1));
	paste->author = dup(sqlite3_column_text(stmt, 2));
	paste->language = dup(sqlite3_column_text(stmt, 3));
	paste->code = dup(sqlite3_column_text(stmt, 4));
	paste->timestamp = sqlite3_column_int64(stmt, 5);
	paste->visible = sqlite3_column_int(stmt, 6);
	paste->duration = sqlite3_column_int64(stmt, 7);
}

static int
exists(const char *id)
{
	assert(id);

	sqlite3_stmt *stmt = NULL;
	int ret = 1;

	if (sqlite3_prepare(db, CHAR(sql_get), -1, &stmt, NULL) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, id, -1, NULL);
		ret = sqlite3_step(stmt) == SQLITE_ROW;
		sqlite3_finalize(stmt);
	}

	return ret;
}

static const char *
create_id(void)
{
	static const char table[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	static char id[12];

	for (size_t i = 0; i < sizeof (id); ++i)
		id[i] = table[rand() % (sizeof (table) - 1)];

	return id;
}

static int
set_id(struct paste *paste)
{
	assert(paste);

	paste->id = NULL;

	/*
	 * Avoid infinite loop, we only try to create a new id in 30 steps.
	 *
	 * On error, the function `exist` returns true to indicate we should
	 * not try to save with that id.
	 */
	int tries = 0;

	do {
		free(paste->id);
		paste->id = estrdup(create_id());
	} while (++tries < 30 && exists(paste->id));

	return tries < 30 ? 0 : -1;
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

	if (sqlite3_exec(db, CHAR(sql_init), NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: unable to initialize %s: %s", path, sqlite3_errmsg(db));
		return -1;
	}

	return 0;
}

int
database_recents(struct paste *pastes, size_t *max)
{
	assert(pastes);
	assert(max);

	sqlite3_stmt *stmt = NULL;
	size_t i = 0;

	memset(pastes, 0, *max * sizeof (struct paste));
	log_debug("database: accessing most recents");

	if (sqlite3_prepare(db, CHAR(sql_recents), -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_int64(stmt, 1, *max) != SQLITE_OK)
		goto sqlite_err;

	for (; i < *max && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		convert(stmt, &pastes[i]);

	log_debug("database: found %zu pastes", i);
	sqlite3_finalize(stmt);
	*max = i;

	return 0;

sqlite_err:
	log_warn("database: error (recents): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	*max = 0;

	return -1;
}

int
database_get(struct paste *paste, const char *id)
{
	assert(paste);
	assert(id);

	sqlite3_stmt* stmt = NULL;
	int found = -1;

	memset(paste, 0, sizeof (struct paste));
	log_debug("database: accessing paste with id: %s", id);

	if (sqlite3_prepare(db, CHAR(sql_get), -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_text(stmt, 1, id, -1, NULL) != SQLITE_OK)
		goto sqlite_err;

	switch (sqlite3_step(stmt)) {
	case SQLITE_ROW:
		convert(stmt, paste);
		found = 0;
		break;
	case SQLITE_MISUSE:
	case SQLITE_ERROR:
		goto sqlite_err;
	default:
		break;
	}

	sqlite3_finalize(stmt);

	return found;

sqlite_err:
	if (stmt)
		sqlite3_finalize(stmt);

	log_warn("database: error (get): %s", sqlite3_errmsg(db));

	return -1;
}

int
database_insert(struct paste *paste)
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
	if (sqlite3_prepare(db, CHAR(sql_insert), -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;

	sqlite3_bind_text(stmt, 1, paste->id, -1, SQLITE_STATIC);
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
	    paste->id, paste->author, paste->duration);

	return 0;

sqlite_err:
	log_warn("database: error (insert): %s", sqlite3_errmsg(db));
	sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);

	if (stmt)
		sqlite3_finalize(stmt);

	free(paste->id);
	paste->id = NULL;

	return -1;
}

int
database_search(struct paste *pastes,
                size_t *max,
                const char *title,
                const char *author,
                const char *language)
{
	assert(pastes);
	assert(max);

	sqlite3_stmt *stmt = NULL;
	size_t i = 0;

	log_debug("database: searching title=%s, author=%s, language=%s",
	    title    ? title    : "",
	    author   ? author   : "",
	    language ? language : "");

	memset(pastes, 0, *max * sizeof (struct paste));

	/* Select everything if not specified. */
	title    = title    ? title    : "%";
	author   = author   ? author   : "%";
	language = language ? language : "%";

	if (sqlite3_prepare(db, CHAR(sql_search), -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 2, author, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 3, language, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_int64(stmt, 4, *max) != SQLITE_OK)
		goto sqlite_err;

	for (; i < *max && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		convert(stmt, &pastes[i]);

	log_debug("database: found %zu pastes", i);
	sqlite3_finalize(stmt);
	*max = i;

	return 0;

sqlite_err:
	log_warn("database: error (search): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	*max = 0;

	return -1;
}

void
database_clear(void)
{
	log_debug("database: clearing deprecated pastes");

	if (sqlite3_exec(db, CHAR(sql_clear), NULL, NULL, NULL) != SQLITE_OK)
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
