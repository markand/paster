#
# paster -- convenient front-end to a pasterd instance
#
# Copyright (c) 2020 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

author="Anonymous"
language="nohighlight"
title="Untitled"
duration="day"
public=0
verbose=0

die()
{
	echo "$@" 1>&2
	exit 1
}

languages()
{
	echo "nohighlight"
	echo "1c"
	echo "abnf"
	echo "accesslog"
	echo "actionscript"
	echo "ada"
	echo "apache"
	echo "applescript"
	echo "arduino"
	echo "armasm"
	echo "asciidoc"
	echo "aspectj"
	echo "autohotkey"
	echo "autoit"
	echo "avrasm"
	echo "awk"
	echo "axapta"
	echo "bash"
	echo "basic"
	echo "bnf"
	echo "brainfuck"
	echo "cal"
	echo "capnproto"
	echo "ceylon"
	echo "clean"
	echo "clojure"
	echo "clojure-repl"
	echo "cmake"
	echo "coffeescript"
	echo "coq"
	echo "cos"
	echo "cpp"
	echo "crmsh"
	echo "crystal"
	echo "cs"
	echo "csp"
	echo "css"
	echo "dart"
	echo "delphi"
	echo "diff"
	echo "django"
	echo "d"
	echo "dns"
	echo "dockerfile"
	echo "dos"
	echo "dsconfig"
	echo "dts"
	echo "dust"
	echo "ebnf"
	echo "elixir"
	echo "elm"
	echo "erb"
	echo "erlang"
	echo "erlang-repl"
	echo "excel"
	echo "fix"
	echo "flix"
	echo "fortran"
	echo "fsharp"
	echo "gams"
	echo "gauss"
	echo "gcode"
	echo "gherkin"
	echo "glsl"
	echo "go"
	echo "golo"
	echo "gradle"
	echo "groovy"
	echo "haml"
	echo "handlebars"
	echo "haskell"
	echo "haxe"
	echo "hsp"
	echo "htmlbars"
	echo "http"
	echo "hy"
	echo "inform7"
	echo "ini"
	echo "irpf90"
	echo "java"
	echo "javascript"
	echo "jboss-cli"
	echo "json"
	echo "julia"
	echo "julia-repl"
	echo "kotlin"
	echo "lasso"
	echo "ldif"
	echo "leaf"
	echo "less"
	echo "lisp"
	echo "livecodeserver"
	echo "livescript"
	echo "llvm"
	echo "lsl"
	echo "lua"
	echo "makefile"
	echo "markdown"
	echo "mathematica"
	echo "matlab"
	echo "maxima"
	echo "mel"
	echo "mercury"
	echo "mipsasm"
	echo "mizar"
	echo "mojolicious"
	echo "monkey"
	echo "moonscript"
	echo "n1ql"
	echo "nginx"
	echo "nimrod"
	echo "nix"
	echo "nsis"
	echo "objectivec"
	echo "ocaml"
	echo "openscad"
	echo "oxygene"
	echo "parser3"
	echo "perl"
	echo "pf"
	echo "php"
	echo "pony"
	echo "powershell"
	echo "processing"
	echo "profile"
	echo "prolog"
	echo "protobuf"
	echo "puppet"
	echo "purebasic"
	echo "python"
	echo "q"
	echo "qml"
	echo "rib"
	echo "r"
	echo "roboconf"
	echo "routeros"
	echo "rsl"
	echo "ruby"
	echo "ruleslanguage"
	echo "rust"
	echo "scala"
	echo "scheme"
	echo "scilab"
	echo "scss"
	echo "shell"
	echo "smali"
	echo "smalltalk"
	echo "sml"
	echo "sqf"
	echo "sql"
	echo "stan"
	echo "stata"
	echo "step21"
	echo "stylus"
	echo "subunit"
	echo "swift"
	echo "taggerscript"
	echo "tap"
	echo "tcl"
	echo "tex"
	echo "thrift"
	echo "tp"
	echo "twig"
	echo "typescript"
	echo "vala"
	echo "vbnet"
	echo "vbscript-html"
	echo "vbscript"
	echo "verilog"
	echo "vhdl"
	echo "vim"
	echo "x86asm"
	echo "xl"
	echo "xml"
	echo "xquery"
	echo "yaml"
	echo "zephir"
	exit 0
}

durations()
{
	echo "hour"
	echo "day"
	echo "week"
	echo "month"
	exit 0
}

usage()
{
	cat 1>&2 <<-EOF
	usage: paster [-LDpv] [-a author] [-l language] [-d duration] [-t title] filename host
	EOF
	exit 1
}

send()
{
	if [ $public -eq 1 ]; then
		with_private="--data private=off"
	else
		with_private="--data private=on"
	fi

	if [ $verbose -eq 0 ]; then
		with_verbose="-s"
	fi

	curl -i -X POST \
		--data author="$author" \
		--data language="$language" \
		--data duration="$duration" \
		--data title="$title" \
		--data-urlencode code@"$1" \
		$with_private \
		$with_verbose \
		"$2"/new
}

if ! command -v curl >/dev/null 2>&1; then
	die "abort: curl is required"
fi

while getopts "LDa:d:l:pt:v" opt; do
	case "$opt" in
	D)
		durations
		;;
	L)
		languages
		;;
	a)
		author="$OPTARG"
		;;
	d)
		duration="$OPTARG"
		;;
	l)
		language="$OPTARG"
		;;
	p)
		public=1
		;;
	t)
		title="$OPTARG"
		;;
	v)
		verbose=1
		;;
	*)
		usage
		;;
	esac
done

shift $((OPTIND - 1))

if [ "$#" -ne 2 ]; then
	usage
fi

# If verbose, dump all headers.
if [ $verbose -eq 1 ]; then
	send "$1" "$2"
else
	url=$(send "$1" "$2" | grep -E "^Location: " | awk '{ print $2 }')

	if [ -z "$url" ]; then
		die "abort: error occured, retry with -v"
	fi

	printf "%s%s\n" "$2" "$url"
fi
