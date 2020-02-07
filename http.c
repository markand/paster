/*
 * http.c -- HTTP parsing and rendering
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

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "config.h"
#include "database.h"
#include "http.h"
#include "log.h"
#include "paste.h"
#include "util.h"

static void page_index(struct kreq *);
static void page_new(struct kreq *);
static void page_fork(struct kreq *);
static void page_paste(struct kreq *);
static void page_download(struct kreq *);
static void page_search(struct kreq *);
static void page_static(struct kreq *);

enum page {
	PAGE_INDEX,
	PAGE_NEW,
	PAGE_FORK,
	PAGE_PASTE,
	PAGE_DOWNLOAD,
	PAGE_SEARCH,
	PAGE_STATIC,
	PAGE_LAST       /* Not used. */
};

static const char *pages[] = {
	[PAGE_INDEX]    = "",
	[PAGE_NEW]      = "new",
	[PAGE_FORK]     = "fork",
	[PAGE_PASTE]    = "paste",
	[PAGE_DOWNLOAD] = "download",
	[PAGE_SEARCH]   = "search",
	[PAGE_STATIC]   = "static"
};

static void (*handlers[])(struct kreq *req) = {
	[PAGE_INDEX]    = page_index,
	[PAGE_NEW]      = page_new,
	[PAGE_FORK]     = page_fork,
	[PAGE_PASTE]    = page_paste,
	[PAGE_DOWNLOAD] = page_download,
	[PAGE_SEARCH]   = page_search,
	[PAGE_STATIC]   = page_static
};

struct tmpl_index {
	struct kreq *req;
	struct paste pastes[10];
	size_t count;
	size_t current;
};

struct tmpl_paste {
	struct kreq *req;
	struct paste paste;
};

static const char *tmpl_index_keywords[] = {
	"pastes"
};

static const char *tmpl_index_pastes_keywords[] = {
	"uuid",
	"name",
	"author",
	"language",
	"expiration",
	"date"
};

static const char *tmpl_paste_keywords[] = {
	"uuid",
	"title",
	"author",
	"language",
	"code",
	"timestamp",
	"visible",
	"duration"
};

static const char *tmpl_new_keywords[] = {
	"title",        /* /fork only */
	"author",       /* /fork only */
	"code",         /* /fork only */
	"private",      /* /fork only */
	"languages",
	"durations"
};

static const char *languages[] = {
	"nohighlight",
	"1c",
	"abnf",
	"accesslog",
	"actionscript",
	"ada",
	"apache",
	"applescript",
	"arduino",
	"armasm",
	"asciidoc",
	"aspectj",
	"autohotkey",
	"autoit",
	"avrasm",
	"awk",
	"axapta",
	"bash",
	"basic",
	"bnf",
	"brainfuck",
	"cal",
	"capnproto",
	"ceylon",
	"clean",
	"clojure",
	"clojure-repl",
	"cmake",
	"coffeescript",
	"coq",
	"cos",
	"cpp",
	"crmsh",
	"crystal",
	"cs",
	"csp",
	"css",
	"dart",
	"delphi",
	"diff",
	"django",
	"d",
	"dns",
	"dockerfile",
	"dos",
	"dsconfig",
	"dts",
	"dust",
	"ebnf",
	"elixir",
	"elm",
	"erb",
	"erlang",
	"erlang-repl",
	"excel",
	"fix",
	"flix",
	"fortran",
	"fsharp",
	"gams",
	"gauss",
	"gcode",
	"gherkin",
	"glsl",
	"go",
	"golo",
	"gradle",
	"groovy",
	"haml",
	"handlebars",
	"haskell",
	"haxe",
	"hsp",
	"htmlbars",
	"http",
	"hy",
	"inform7",
	"ini",
	"irpf90",
	"java",
	"javascript",
	"jboss-cli",
	"json",
	"julia",
	"julia-repl",
	"kotlin",
	"lasso",
	"ldif",
	"leaf",
	"less",
	"lisp",
	"livecodeserver",
	"livescript",
	"llvm",
	"lsl",
	"lua",
	"makefile",
	"markdown",
	"mathematica",
	"matlab",
	"maxima",
	"mel",
	"mercury",
	"mipsasm",
	"mizar",
	"mojolicious",
	"monkey",
	"moonscript",
	"n1ql",
	"nginx",
	"nimrod",
	"nix",
	"nsis",
	"objectivec",
	"ocaml",
	"openscad",
	"oxygene",
	"parser3",
	"perl",
	"pf",
	"php",
	"pony",
	"powershell",
	"processing",
	"profile",
	"prolog",
	"protobuf",
	"puppet",
	"purebasic",
	"python",
	"q",
	"qml",
	"rib",
	"r",
	"roboconf",
	"routeros",
	"rsl",
	"ruby",
	"ruleslanguage",
	"rust",
	"scala",
	"scheme",
	"scilab",
	"scss",
	"shell",
	"smali",
	"smalltalk",
	"sml",
	"sqf",
	"sql",
	"stan",
	"stata",
	"step21",
	"stylus",
	"subunit",
	"swift",
	"taggerscript",
	"tap",
	"tcl",
	"tex",
	"thrift",
	"tp",
	"twig",
	"typescript",
	"vala",
	"vbnet",
	"vbscript-html",
	"vbscript",
	"verilog",
	"vhdl",
	"vim",
	"x86asm",
	"xl",
	"xml",
	"xquery",
	"yaml",
	"zephir",
	NULL
};

static const struct {
	const char *title;
	long long int secs;
} durations[] = {
	{ "month",      PASTE_MONTH     },
	{ "week",       PASTE_WEEK      },
	{ "day",        PASTE_DAY       },
	{ "hour",       PASTE_HOUR      },
	{ NULL,         -1              }
};

static const char *
template(const char *filename)
{
	/* Build path to the template file. */
	static char path[PATH_MAX];

	snprintf(path, sizeof (path), "%s/%s", config.themedir, filename);

	return path;
}

static long long int
duration(const char *val)
{
	if (strcmp(val, "hour") == 0)
		return PASTE_HOUR;
	if (strcmp(val, "day") == 0)
		return PASTE_DAY;
	if (strcmp(val, "week") == 0)
		return PASTE_WEEK;
	if (strcmp(val, "month") == 0)
		return PASTE_MONTH;

	/* Default to month. */
	return PASTE_MONTH;
}

static const char *
ttl(time_t timestamp, long long int duration)
{
	const time_t now = time(NULL);
	const long long int left = duration - difftime(now, timestamp);

	if (left < PASTE_HOUR)
		return bprintf("%lld minute(s)", left / 60);
	if (left < PASTE_DAY)
		return bprintf("%lld hour(s)", left / 3600);

	/* Other in days. */
	return bprintf("%lld day(s)", left / 86400);
}

static void
replace(char **dst, const char *s)
{
	assert(dst);
	assert(s);

	/* Trim leading spaces. */
	while (*s && isspace(*s))
		s++;

	if (*s) {
		free(*dst);
		*dst = estrdup(s);
	}
}

static void
render_languages(struct kreq *req, const struct paste *paste)
{
	for (const char **l = languages; *l != NULL; ++l) {
		const char *line;

		if (paste->language && strcmp(paste->language, *l) == 0)
			line = bprintf("<option value=\"%s\" selected>%s</option>", *l, *l);
		else
			line = bprintf("<option value=\"%s\">%s</option>", *l, *l);

		khttp_puts(req, line);
	}
}

static void
render_durations(struct kreq *req, const struct paste *paste)
{
	for (size_t i = 0; durations[i].title != NULL; ++i) {
		const char *line;

		if (paste->duration == durations[i].secs)
			line = bprintf("<option value=\"%s\" selected>%s</option>",
			    durations[i].title, durations[i].title);
		else
			line = bprintf("<option value=\"%s\">%s</option>",
			    durations[i].title, durations[i].title);

		khttp_puts(req, line);
	}
}

static int
tmpl_paste(size_t index, void *arg)
{
	struct tmpl_paste *data = arg;
	struct paste *paste = &data->paste;
	struct khtmlreq htmlreq;

	khtml_open(&htmlreq, data->req, KHTML_PRETTY);

	switch (index) {
	case 0:
		khtml_puts(&htmlreq, paste->uuid);
		break;
	case 1:
		khtml_puts(&htmlreq, paste->title);
		break;
	case 2:
		khtml_puts(&htmlreq, paste->author);
		break;
	case 3:
		khtml_puts(&htmlreq, paste->language);
		break;
	case 4:
		khtml_puts(&htmlreq, paste->code);
		break;
	case 5:
		khtml_puts(&htmlreq, bstrftime("%c", localtime(&paste->timestamp)));
		break;
	case 6:
		khtml_puts(&htmlreq, bprintf("%s", paste->visible ? "Yes" : "No"));
		break;
	case 7:
		khtml_puts(&htmlreq, ttl(paste->timestamp, paste->duration));
		break;
	default:
		break;
	}

	khtml_close(&htmlreq);

	return true;
}

static int
tmpl_index_pastes(size_t index, void *arg)
{
	struct tmpl_index *data = arg;
	struct paste *paste = &data->pastes[data->current];
	struct khtmlreq htmlreq;

	khtml_open(&htmlreq, data->req, KHTML_PRETTY);

	switch (index) {
	case 0:
		khtml_puts(&htmlreq, paste->uuid);
		break;
	case 1:
		khtml_puts(&htmlreq, paste->title);
		break;
	case 2:
		khtml_puts(&htmlreq, paste->author);
		break;
	case 3:
		khtml_puts(&htmlreq, paste->language);
		break;
	case 4:
		khtml_puts(&htmlreq, ttl(paste->timestamp, paste->duration));
		break;
	case 5:
		khtml_puts(&htmlreq, bstrftime("%c", localtime(&paste->timestamp)));
		break;
	default:
		break;
	}

	khtml_close(&htmlreq);

	return true;
}

static int
tmpl_index(size_t index, void *arg)
{
	/* No check, only one index. */
	(void)index;

	struct tmpl_index *data = arg;
	const struct ktemplate kt = {
		.key    = tmpl_index_pastes_keywords,
		.keysz  = 6,
		.arg    = data,
		.cb     = tmpl_index_pastes
	};

	for (size_t i = 0; i < data->count; ++i) {
		khttp_template(data->req, &kt, template("index-paste.html"));
		data->current++;
	}

	return true;
}

static int
tmpl_new(size_t index, void *arg)
{
	struct tmpl_paste *data = arg;
	struct paste *paste = &data->paste;
	struct khtmlreq htmlreq;

	khtml_open(&htmlreq, data->req, KHTML_PRETTY);

	switch (index) {
	case 0:
		if (paste->title)
			khtml_puts(&htmlreq, paste->title);
		break;
	case 1:
		if (paste->author)
			khtml_puts(&htmlreq, paste->author);
		break;
	case 2:
		if (paste->code)
			khtml_puts(&htmlreq, paste->code);
		break;
	case 3:
		/* Add checked attribute to combobox. */
		if (!paste->visible)
			khttp_puts(data->req, "checked");
		break;
	case 4:
		render_languages(data->req, paste);
		break;
	case 5:
		render_durations(data->req, paste);
		break;
	default:
		break;
	};

	khtml_close(&htmlreq);

	return true;
}

static void
page_header(struct kreq *req)
{
	khttp_template(req, NULL, template("header.html"));
}

static void
page_footer(struct kreq *req)
{
	khttp_template(req, NULL, template("footer.html"));
}

static void
page(struct kreq *req, const struct ktemplate *tmpl, enum khttp status, const char *file)
{
	khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_body(req);
	page_header(req);
	khttp_template(req, tmpl, template(file));
	page_footer(req);
	khttp_free(req);
}

static void
page_index_get(struct kreq *req)
{
	struct tmpl_index data = {
		.req    = req,
		.count  = 10
	};

	if (!database_recents(data.pastes, &data.count))
		page(req, NULL, KHTTP_500, "500.html");
	else {
		struct ktemplate kt = {
			.key    = tmpl_index_keywords,
			.keysz  = 1,
			.arg    = &data,
			.cb     = tmpl_index
		};

		page(req, &kt, KHTTP_200, "index.html");
	}

	for (size_t i = 0; i < data.count; ++i)
		paste_finish(&data.pastes[i]);
}

static void
page_index(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_index_get(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}

static void
page_new_get(struct kreq *req)
{
	struct tmpl_paste data = {
		.req    = req
	};
	const struct ktemplate kt = {
		.key    = tmpl_new_keywords,
		.keysz  = 6,
		.cb     = tmpl_new,
		.arg    = &data
	};

	page(req, &kt, KHTTP_200, "new.html");
}

static void
page_new_post(struct kreq *req)
{
	struct paste paste = {
		.author         = estrdup("Anonymous"),
		.title          = estrdup("Untitled"),
		.language       = estrdup("nohighlight"),
		.visible        = true
	};

	for (size_t i = 0; i < req->fieldsz; ++i) {
		const char *key = req->fields[i].key;
		const char *val = req->fields[i].val;

		if (strcmp(key, "title") == 0)
			replace(&paste.title, val);
		else if (strcmp(key, "author") == 0)
			replace(&paste.author, val);
		else if (strcmp(key, "language") == 0)
			replace(&paste.language, val);
		else if (strcmp(key, "duration") == 0)
			paste.duration = duration(val);
		else if (strcmp(key, "code") == 0)
			paste.code = estrdup(val);
		else if (strcmp(key, "private") == 0)
			paste.visible = strcmp(val, "on") != 0;
	}

	/* Add empty string if needed. */
	if (!paste.code)
		paste.code = estrdup("");

	if (!database_insert(&paste))
		page(req, NULL, KHTTP_500, "500.html");
	else {
		/* Redirect to paste details. */
		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
		khttp_head(req, kresps[KRESP_LOCATION], "/paste/%s", paste.uuid);
		khttp_body(req);
		khttp_free(req);
	}

	paste_finish(&paste);
}

static void
page_new(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_new_get(req);
		break;
	case KMETHOD_POST:
		page_new_post(req);
		break;
	default:
		break;
	}
}

static void
page_fork_get(struct kreq *req)
{
	struct tmpl_paste data = {
		.req = req
	};

	if (!database_get(&data.paste, req->path))
		page(req, NULL, KHTTP_404, "404.html");
	else {
		const struct ktemplate kt = {
			.key    = tmpl_new_keywords,
			.keysz  = 6,
			.cb     = tmpl_new,
			.arg    = &data
		};

		page(req, &kt, KHTTP_200, "new.html");
		paste_finish(&data.paste);
	}
}

static void
page_fork(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_fork_get(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}

static void
page_paste_get(struct kreq *req)
{
	struct tmpl_paste data = {
		.req = req
	};

	if (!database_get(&data.paste, req->path))
		page(req, NULL, KHTTP_404, "404.html");
	else {
		const struct ktemplate kt = {
			.key    = tmpl_paste_keywords,
			.keysz  = 8,
			.cb     = tmpl_paste,
			.arg    = &data
		};

		page(req, &kt, KHTTP_200, "paste.html");
		paste_finish(&data.paste);
	}
}

static void
page_paste(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_paste_get(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}

static void
page_download_get(struct kreq *req)
{
	struct paste paste;

	if (!database_get(&paste, req->path))
		page(req, NULL, KHTTP_404, "404.html");
	else {
		khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
#if 0
		/* TODO: this seems to generated truncated files. */
		khttp_head(req, kresps[KRESP_CONTENT_LENGTH], "%zu", strlen(paste.code));
#endif
		khttp_head(req, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(req, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s.%s\"", paste.uuid, paste.language);
		khttp_body(req);
		khttp_puts(req, paste.code);
		khttp_free(req);
		paste_finish(&paste);
	}
}

static void
page_download(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_download_get(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}

static void
page_search_get(struct kreq *req)
{
	/* We re-use the /new form with an empty paste. */
	struct tmpl_paste data = {
		.req = req
	};
	const struct ktemplate kt = {
		.key    = tmpl_new_keywords,
		.keysz  = 6,
		.cb     = tmpl_new,
		.arg    = &data
	};

	page(req, &kt, KHTTP_200, "search.html");
}

static void
page_search_post(struct kreq *req)
{
	struct tmpl_index data = {
		.req    = req,
		.count  = 10
	};

	const char *title = NULL;
	const char *author = NULL;
	const char *language = NULL;

	for (size_t i = 0; i < req->fieldsz; ++i) {
		const char *key = req->fields[i].key;
		const char *val = req->fields[i].val;

		if (strcmp(key, "title") == 0)
			title = val;
		else if (strcmp(key, "author") == 0)
			author = val;
		else if (strcmp(key, "language") == 0)
			language = val;
	}

	/* Sets to null if they are empty. */
	if (title && strlen(title) == 0)
		title = NULL;
	if (author && strlen(author) == 0)
		author = NULL;
	if (language && strlen(language) == 0)
		language = NULL;

	if (!database_search(data.pastes, &data.count, title, author, language))
		page(req, NULL, KHTTP_500, "500.html");
	else {
		struct ktemplate kt = {
			.key    = tmpl_index_keywords,
			.keysz  = 1,
			.arg    = &data,
			.cb     = tmpl_index
		};

		page(req, &kt, KHTTP_200, "index.html");
	}

	for (size_t i = 0; i < data.count; ++i)
		paste_finish(&data.pastes[i]);
}

static void
page_search(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_search_get(req);
		break;
	case KMETHOD_POST:
		page_search_post(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}

static void
page_static_get(struct kreq *req)
{
	struct stat st;
	char path[PATH_MAX];

	snprintf(path, sizeof (path), "%s%s", config.themedir, req->fullpath);

	if (stat(path, &st) < 0)
		page(req, NULL, KHTTP_404, "404.html");
	else {
		khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_head(req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[req->mime]);
		khttp_head(req, kresps[KRESP_CONTENT_LENGTH],
		    "%llu", (unsigned long long)(st.st_size));
		khttp_body(req);
		khttp_template(req, NULL, path);
		khttp_free(req);
	}
}

static void
page_static(struct kreq *req)
{
	switch (req->method) {
	case KMETHOD_GET:
		page_static_get(req);
		break;
	default:
		page(req, NULL, KHTTP_400, "400.html");
		break;
	}
}

static void
process(struct kreq *req)
{
	assert(req);

	log_debug("http: accessing page '%s'", req->path);

	if (req->page == PAGE_LAST)
		page(req, NULL, KHTTP_404, "404.html");
	else
		handlers[req->page](req);
}

void
http_fcgi_run(void)
{
	struct kreq req;
	struct kfcgi *fcgi;

	if (khttp_fcgi_init(&fcgi, NULL, 0, pages, PAGE_LAST, 0) != KCGI_OK)
		return;

	while (khttp_fcgi_parse(fcgi, &req) == KCGI_OK)
		process(&req);

	khttp_fcgi_free(fcgi);
}

void
http_cgi_run(void)
{
	struct kreq req;

	if (khttp_parse(&req, NULL, 0, pages, PAGE_LAST, 0) == KCGI_OK)
		process(&req);
}
