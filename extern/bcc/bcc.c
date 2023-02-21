/*
 * bcc.c -- binary to C/C++ arrays converter
 *
 * Copyright (c) 2020-2021 David Demelier <markand@malikania.fr>
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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

static const char *charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
static char findentchar = '\t';
static int findent = 1, fconst, fnull, fstatic;

noreturn static void
usage(void)
{
	fprintf(stderr, "usage: bcc [-0cs] [-I tab-indent] [-i space-indent] input variable\n");
	exit(1);
}

noreturn static void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fputs("abort: ", stderr);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

static char *
mangle(char *variable)
{
	char *p;
	size_t pos;

	/* Remove extension. */
	if ((p = strrchr(variable, '.')))
		*p = '\0';

	/* Remove disallowed characters. */
	while ((pos = strspn(variable, charset)) != strlen(variable))
		variable[pos] = '_';

	return variable;
}

static void
indent(void)
{
	for (int i = 0; i < findent; ++i)
		putchar(findentchar);
}

static void
put(int ch)
{
	printf("0x%02hhx", (unsigned char)ch);
}

static void
process(const char *input, const char *variable)
{
	FILE *fp;
	int ch, col = 0;

	if (strcmp(input, "-") == 0)
		fp = stdin;
	else if (!(fp = fopen(input, "rb")))
		die("%s: %s\n", input, strerror(errno));

	if (fstatic)
		printf("static ");
	if (fconst)
		printf("const ");

	printf("unsigned char %s[] = {\n", variable);

	for (ch = fgetc(fp); ch != EOF; ) {
		if (col == 0)
			indent();

		put(ch);

		if ((ch = fgetc(fp)) != EOF || fnull)
			printf(",%s", col < 3 ? " " : "");

		if (++col == 4) {
			col = 0;
			putchar('\n');
		}

		/* Add final '\0' if required. */
		if (ch == EOF && fnull) {
			if (col++ == 0)
				indent();

			put(0);
		}
	}

	if (col != 0)
		printf("\n");

	puts("};");
	fclose(fp);
}

int
main(int argc, char **argv)
{
	int ch;

	while ((ch = getopt(argc, argv, "0cI:i:s")) != -1) {
		switch (ch) {
		case '0':
			fnull = 1;
			break;
		case 'c':
			fconst = 1;
			break;
		case 'I':
			findentchar = '\t';
			findent = atoi(optarg);
			break;
		case 'i':
			findentchar = ' ';
			findent = atoi(optarg);
			break;
		case 's':
			fstatic = 1;
			break;
		default:
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	process(argv[0], mangle(argv[1]));
}
