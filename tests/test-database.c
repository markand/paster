/*
 * test-database.c -- test database functions
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
#include <unistd.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include "database.h"
#include "paste.h"
#include "util.h"

#define TEST_DATABASE "test.db"

static void
setup(void *data)
{
	remove(TEST_DATABASE);

	if (!database_open(TEST_DATABASE))
		die("abort: could not open database");

	(void)data;
}

static void
finish(void *data)
{
	database_finish();

	(void)data;
}

GREATEST_TEST
recents_empty(void)
{
	struct paste pastes[10];
	size_t max = 10;

	if (!database_recents(pastes, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_PASS();
}

GREATEST_TEST
recents_one(void)
{
	struct paste pastes[10];
	size_t max = 10;
	struct paste one = {
		.title = estrdup("test 1"),
		.author = estrdup("unit test"),
		.language = estrdup("cpp"),
		.code = estrdup("int main() {}"),
		.duration = PASTE_HOUR,
		.visible = true
	};

	if (!database_insert(&one))
		GREATEST_FAIL();
	if (!database_recents(pastes, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 1);
	GREATEST_ASSERT(pastes[0].uuid);
	GREATEST_ASSERT_STR_EQ(pastes[0].title, "test 1");
	GREATEST_ASSERT_STR_EQ(pastes[0].author, "unit test");
	GREATEST_ASSERT_STR_EQ(pastes[0].language, "cpp");
	GREATEST_ASSERT_STR_EQ(pastes[0].code, "int main() {}");
	GREATEST_ASSERT_EQ(pastes[0].duration, PASTE_HOUR);
	GREATEST_ASSERT(pastes[0].visible);
	GREATEST_PASS();
}


GREATEST_TEST
recents_hidden(void)
{
	struct paste pastes[10];
	size_t max = 10;
	struct paste one = {
		.title = estrdup("test 1"),
		.author = estrdup("unit test"),
		.language = estrdup("cpp"),
		.code = estrdup("int main() {}"),
		.duration = PASTE_HOUR,
		.visible = false
	};

	if (!database_insert(&one))
		GREATEST_FAIL();
	if (!database_recents(pastes, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_PASS();
}

GREATEST_TEST
recents_many(void)
{
	static const int expected[] = { 2, 1, 0 };
	struct paste pastes[3];
	size_t max = 3;
	struct paste pastie = {
		.duration = PASTE_HOUR,
		.visible = true
	};

	for (int i = 0; i < 3; ++i) {
		pastie.title = estrdup(bprintf("test %d", i));
		pastie.author = estrdup(bprintf("unit test %d", i));
		pastie.language = estrdup("cpp");
		pastie.code = estrdup(bprintf("int main() { return %d; }", i));

		if (!database_insert(&pastie))
			GREATEST_FAIL();

		/* Sleep a little bit to avoid same timestamp. */
		sleep(2);
	};

	if (!database_recents(pastes, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 3U);

	for (int i = 0; i < 3; ++i) {
		/* Selected in most recents first. */
		GREATEST_ASSERT(pastes[i].uuid);
		GREATEST_ASSERT_STR_EQ(pastes[i].title,
		    bprintf("test %d", expected[i]));
		GREATEST_ASSERT_STR_EQ(pastes[i].author,
		    bprintf("unit test %d", expected[i]));
		GREATEST_ASSERT_STR_EQ(pastes[i].language, "cpp");
		GREATEST_ASSERT_STR_EQ(pastes[i].code,
		    bprintf("int main() { return %d; }", expected[i]));
		GREATEST_ASSERT_EQ(pastes[i].duration, PASTE_HOUR);
		GREATEST_ASSERT(pastes[i].visible);
	};

	GREATEST_PASS();
}

GREATEST_TEST
recents_limits(void)
{
	static const int expected[] = { 19, 18, 17 };
	struct paste pastes[3];
	size_t max = 3;
	struct paste pastie = {
		.duration = PASTE_HOUR,
		.visible = true
	};

	for (int i = 0; i < 20; ++i) {
		pastie.title = estrdup(bprintf("test %d", i));
		pastie.author = estrdup(bprintf("unit test %d", i));
		pastie.language = estrdup("cpp");
		pastie.code = estrdup(bprintf("int main() { return %d; }", i));

		if (!database_insert(&pastie))
			GREATEST_FAIL();

		/* Sleep a little bit to avoid same timestamp. */
		sleep(2);
	};

	if (!database_recents(pastes, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 3U);

	for (int i = 0; i < 3; ++i) {
		/* Selected in most recents first. */
		GREATEST_ASSERT(pastes[i].uuid);
		GREATEST_ASSERT_STR_EQ(pastes[i].title,
		    bprintf("test %d", expected[i]));
		GREATEST_ASSERT_STR_EQ(pastes[i].author,
		    bprintf("unit test %d", expected[i]));
		GREATEST_ASSERT_STR_EQ(pastes[i].language, "cpp");
		GREATEST_ASSERT_STR_EQ(pastes[i].code,
		    bprintf("int main() { return %d; }", expected[i]));
		GREATEST_ASSERT_EQ(pastes[i].duration, PASTE_HOUR);
		GREATEST_ASSERT(pastes[i].visible);
	};

	GREATEST_PASS();
}

GREATEST_SUITE(recents)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(recents_empty);
	GREATEST_RUN_TEST(recents_one);
	GREATEST_RUN_TEST(recents_hidden);
	GREATEST_RUN_TEST(recents_many);
	GREATEST_RUN_TEST(recents_limits);
}

GREATEST_TEST
get_basic(void)
{
	struct paste original = {
		.title = estrdup("test 1"),
		.author = estrdup("unit test"),
		.language = estrdup("cpp"),
		.code = estrdup("int main() {}"),
		.duration = PASTE_HOUR,
		.visible = false
	};
	struct paste new = { 0 };

	if (!database_insert(&original))
		GREATEST_FAIL();
	if (!database_get(&new, original.uuid))
		GREATEST_FAIL();

	GREATEST_ASSERT_STR_EQ(new.uuid, original.uuid);
	GREATEST_ASSERT_STR_EQ(new.title, original.title);
	GREATEST_ASSERT_STR_EQ(new.author, original.author);
	GREATEST_ASSERT_STR_EQ(new.language, original.language);
	GREATEST_ASSERT_STR_EQ(new.code, original.code);
	GREATEST_ASSERT_EQ(new.visible, original.visible);
	GREATEST_PASS();
}

GREATEST_TEST
get_nonexistent(void)
{
	struct paste new = { 0 };

	if (!database_get(&new, "unknown"))
		GREATEST_FAIL();

	GREATEST_ASSERT(!new.uuid);
	GREATEST_ASSERT(!new.title);
	GREATEST_ASSERT(!new.author);
	GREATEST_ASSERT(!new.language);
	GREATEST_ASSERT(!new.code);
	GREATEST_PASS();
}

GREATEST_SUITE(get)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(get_basic);
	GREATEST_RUN_TEST(get_nonexistent);
}

GREATEST_TEST
search_basic(void)
{
	struct paste searched[3] = { 0 };
	struct paste originals[] = {
		{
			.title = estrdup("This is in C"),
			.author = estrdup("markand"),
			.language = estrdup("cpp"),
			.code = estrdup("int main(void) {}"),
			.duration = PASTE_HOUR,
			.visible = true
		},
		{
			.title = estrdup("This is in shell"),
			.author = estrdup("markand"),
			.language = estrdup("shell"),
			.code = estrdup("f() {}"),
			.duration = PASTE_HOUR,
			.visible = true
		},
		{
			.title = estrdup("This is in python"),
			.author = estrdup("NiReaS"),
			.language = estrdup("python"),
			.code = estrdup("f: pass"),
			.duration = PASTE_HOUR,
			.visible = true
		},
	};
	size_t max = 3;

	for (int i = 0; i < 3; ++i)
		if (!database_insert(&originals[i]))
			GREATEST_FAIL();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = markand,
	 * language = cpp
	 */
	if (!database_search(searched, &max, NULL, "markand", "cpp"))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 1);
	GREATEST_ASSERT(searched[0].uuid);
	GREATEST_ASSERT_STR_EQ(searched[0].title, "This is in C");
	GREATEST_ASSERT_STR_EQ(searched[0].author, "markand");
	GREATEST_ASSERT_STR_EQ(searched[0].language, "cpp");
	GREATEST_ASSERT_STR_EQ(searched[0].code, "int main(void) {}");
	GREATEST_ASSERT_EQ(searched[0].duration, PASTE_HOUR);
	GREATEST_ASSERT(searched[0].visible);
	GREATEST_PASS();
}

GREATEST_TEST
search_notfound(void)
{
	struct paste searched = { 0 };
	struct paste original = {
		.title = estrdup("This is in C"),
		.author = estrdup("markand"),
		.language = estrdup("cpp"),
		.code = estrdup("int main(void) {}"),
		.duration = PASTE_HOUR,
		.visible = true
	};
	size_t max = 1;

	if (!database_insert(&original))
		GREATEST_FAIL();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = jean,
	 * language = <any>
	 */
	if (!database_search(&searched, &max, NULL, "jean", NULL))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_ASSERT(!searched.uuid);
	GREATEST_ASSERT(!searched.title);
	GREATEST_ASSERT(!searched.author);
	GREATEST_ASSERT(!searched.language);
	GREATEST_ASSERT(!searched.code);
	GREATEST_PASS();
}

GREATEST_TEST
search_private(void)
{
	struct paste searched = { 0 };
	struct paste original = {
		.title = estrdup("This is secret"),
		.author = estrdup("anonymous"),
		.language = estrdup("nohighlight"),
		.code = estrdup("I love you, honey"),
		.duration = PASTE_HOUR,
		.visible = false
	};
	size_t max = 1;

	if (!database_insert(&original))
		GREATEST_FAIL();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = <any>
	 * language = <any>
	 */
	if (!database_search(&searched, &max, NULL, NULL, NULL))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_ASSERT(!searched.uuid);
	GREATEST_ASSERT(!searched.title);
	GREATEST_ASSERT(!searched.author);
	GREATEST_ASSERT(!searched.language);
	GREATEST_ASSERT(!searched.code);
	GREATEST_PASS();
}

GREATEST_SUITE(search)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(search_basic);
	GREATEST_RUN_TEST(search_notfound);
	GREATEST_RUN_TEST(search_private);
}

GREATEST_TEST
clear_run(void)
{
	struct paste searched = { 0 };
	struct paste originals[] = {
		/* Will be deleted */
		{
			.title = estrdup("This is in C"),
			.author = estrdup("markand"),
			.language = estrdup("cpp"),
			.code = estrdup("int main(void) {}"),
			.duration = 1,
			.visible = true
		},
		/* Will be deleted */
		{
			.title = estrdup("This is in shell"),
			.author = estrdup("markand"),
			.language = estrdup("shell"),
			.code = estrdup("f() {}"),
			.duration = 1,
			.visible = true
		},
		/* Will be kept */
		{
			.title = estrdup("This is in python"),
			.author = estrdup("NiReaS"),
			.language = estrdup("python"),
			.code = estrdup("f: pass"),
			.duration = PASTE_HOUR,
			.visible = true
		},
	};
	size_t max = 1;

	for (int i = 0; i < 3; ++i)
		if (!database_insert(&originals[i]))
			GREATEST_FAIL();

	/* Sleep 2 seconds to exceed the lifetime of C and shell pastes. */
	sleep(2);
	database_clear();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = <any>
	 * language = <any>
	 */
	if (!database_search(&searched, &max, NULL, NULL, NULL))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 1);
	GREATEST_ASSERT(searched.uuid);
	GREATEST_ASSERT_STR_EQ(searched.title, "This is in python");
	GREATEST_ASSERT_STR_EQ(searched.author, "NiReaS");
	GREATEST_ASSERT_STR_EQ(searched.language, "python");
	GREATEST_ASSERT_STR_EQ(searched.code, "f: pass");
	GREATEST_ASSERT_EQ(searched.duration, PASTE_HOUR);
	GREATEST_ASSERT(searched.visible);
	GREATEST_PASS();
}

GREATEST_SUITE(clear)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(clear_run);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(recents);
	GREATEST_RUN_SUITE(get);
	GREATEST_RUN_SUITE(search);
	GREATEST_RUN_SUITE(clear);
	GREATEST_MAIN_END();
}
