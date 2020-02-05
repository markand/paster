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
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kcgi.h>

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

enum page {
	PAGE_INDEX,
	PAGE_NEW,
	PAGE_FORK,
	PAGE_PASTE,
	PAGE_DOWNLOAD,
	PAGE_LAST       /* Not used. */
};

static const char *pages[] = {
	[PAGE_INDEX]    = "",
	[PAGE_NEW]      = "new",
	[PAGE_FORK]     = "fork",
	[PAGE_PASTE]    = "paste",
	[PAGE_DOWNLOAD] = "download",
};

static void (*handlers[])(struct kreq *req) = {
	[PAGE_INDEX]    = page_index,
	[PAGE_NEW]      = page_new,
	[PAGE_FORK]     = page_fork,
	[PAGE_PASTE]    = page_paste,
	[PAGE_DOWNLOAD] = page_download
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
	"languages"
};

static const char *languages[] = {
	"nohighlight"
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

static int
tmpl_paste(size_t index, void *arg)
{
	struct tmpl_paste *data = arg;
	struct paste *paste = &data->paste;

	switch (index) {
	case 0:
		khttp_puts(data->req, paste->uuid);
		break;
	case 1:
		khttp_puts(data->req, paste->title);
		break;
	case 2:
		khttp_puts(data->req, paste->author);
		break;
	case 3:
		khttp_puts(data->req, paste->language);
		break;
	case 4:
		khttp_puts(data->req, paste->code);
		break;
	case 5:
		khttp_puts(data->req, bstrftime("%c", localtime(&paste->timestamp)));
		break;
	case 6:
		khttp_puts(data->req, bprintf("%s", paste->visible ? "Yes" : "No"));
		break;
	case 7:
		/* TODO: convert time left. */
		khttp_puts(data->req, "TODO");
		break;
	default:
		break;
	}

	return true;
}

static int
tmpl_index_pastes(size_t index, void *arg)
{
	struct tmpl_index *data = arg;
	struct paste *paste = &data->pastes[data->current];

	switch (index) {
	case 0:
		khttp_puts(data->req, paste->uuid);
		break;
	case 1:
		khttp_puts(data->req, paste->title);
		break;
	case 2:
		khttp_puts(data->req, paste->author);
		break;
	case 3:
		khttp_puts(data->req, paste->language);
		break;
	case 4:
		khttp_puts(data->req, bprintf("%d", paste->duration));
		break;
	case 5:
		khttp_puts(data->req, bstrftime("%c", localtime(&paste->timestamp)));
		break;
	default:
		break;
	}

	return true;
}

static int
tmpl_index(size_t index, void *arg)
{
	/* No check, only one index. */
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

	switch (index) {
	case 0:
		if (paste->title)
			khttp_puts(data->req, paste->title);
		break;
	case 1:
		if (paste->author)
			khttp_puts(data->req, paste->author);
		break;
	case 2:
		if (paste->code)
			khttp_puts(data->req, paste->code);
		break;
	case 3:
		/* TODO: fragment? */
		for (const char **l = languages; *l != NULL; ++l)
			khttp_puts(data->req,
			    bprintf("<option value=\"%s\">%s</option>", *l, *l));
		break;
	default:
		break;
	};

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
		.keysz  = 4,
		.cb     = tmpl_new,
		.arg    = &data
	};

	page(req, &kt, KHTTP_200, "new.html");
}

static void
page_new_post(struct kreq *req)
{
	struct paste paste = {
		.visible = true
	};

	for (size_t i = 0; i < req->fieldsz; ++i) {
		const char *key = req->fields[i].key;
		const char *val = req->fields[i].val;

		if (strcmp(key, "title") == 0)
			paste.title = estrdup(val);
		else if (strcmp(key, "author") == 0)
			paste.author = estrdup(val);
		else if (strcmp(key, "language") == 0)
			paste.language = estrdup(val);
		else if (strcmp(key, "duration") == 0)
			paste.duration = duration(val);
		else if (strcmp(key, "code") == 0)
			paste.code = estrdup(val);
		else if (strcmp(key, "private") == 0)
			paste.visible = strcmp(val, "on") != 0;
	}

	if (!paste.title || !paste.author || !paste.language || !paste.code)
		page(req, NULL, KHTTP_400, "400.html");
	else {
		if (!database_insert(&paste))
			page(req, NULL, KHTTP_500, "500.html");
		else {
			/* Redirect to paste details. */
			khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
			khttp_head(req, kresps[KRESP_LOCATION], "/paste/%s", paste.uuid);
			khttp_body(req);
			khttp_free(req);
		}
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
			.keysz  = 4,
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
process(struct kreq *req)
{
	assert(req);

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
