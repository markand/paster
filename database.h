#ifndef PASTER_DATABASE_H
#define PASTER_DATABASE_H

#include <stdbool.h>
#include <stddef.h>

struct paste;

bool
database_open(const char *path);

bool
database_recents(struct paste *pastes, size_t *max);

bool
database_get(struct paste *paste, const char *uuid);

bool
database_insert(struct paste *paste);

void
database_clear(void);

void
database_finish(void);

#endif /* !PASTER_DATABASE_H */
