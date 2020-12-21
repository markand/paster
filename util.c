/*
 * util.c -- various utilities
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
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "util.h"
#include "paste.h"

const char *languages[] = {
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
	"zephir"
};

const size_t languagesz = NELEM(languages);

noreturn void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
}

char *
estrdup(const char *str)
{
	assert(str);

	char *ret;
	size_t length = strlen(str);

	if (!(ret = calloc(1, length + 1)))
		die(strerror(errno));

	return strcpy(ret, str);
}

const char *
bprintf(const char *fmt, ...)
{
	static char buf[BUFSIZ];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	return buf;
}

const char *
bstrftime(const char *fmt, const struct tm *tm)
{
	static char buf[BUFSIZ];

	strftime(buf, sizeof (buf), fmt, tm);

	return buf;
}

const char *
path(const char *filename)
{
	assert(filename);

	/* Build path to the template file. */
	static char path[PATH_MAX];

	snprintf(path, sizeof (path), "%s/%s", config.themedir, filename);

	return path;
}

void
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

const char *
ttl(time_t timestamp, long long int duration)
{
	const time_t now = time(NULL);
	const long long int left = duration - difftime(now, timestamp);

	if (left < PASTE_DURATION_HOUR)
		return bprintf("%lld minute(s)", left / 60);
	if (left < PASTE_DURATION_DAY)
		return bprintf("%lld hour(s)", left / 3600);

	/* Other in days. */
	return bprintf("%lld day(s)", left / 86400);
}
