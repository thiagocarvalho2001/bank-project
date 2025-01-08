#ifndef DB_H
#define DB_H

#include <postgresql/libpq-fe.h>

PGconn *connect_to_db();

#endif