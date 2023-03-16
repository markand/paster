--
-- init.sql -- database initialization
--
-- Copyright (c) 2020-2023 David Demelier <markand@malikania.fr>
--
-- Permission to use, copy, modify, and/or distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
--

BEGIN EXCLUSIVE TRANSACTION;

CREATE TABLE IF NOT EXISTS paste(
	`id`            TEXT primary key,
	`title`         TEXT not null,
	`author`        TEXT not null,
	`language`      TEXT not null,
	`code`          TEXT not null,
	`date`          INT default CURRENT_TIMESTAMP,
	`visible`       INT default 0,
	`duration`      INT
);

END TRANSACTION;
