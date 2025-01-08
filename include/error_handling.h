#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <postgresql/libpq-fe.h>

void finish_with_error(PGconn *conn);
int check_query_status(PGconn *conn, const char *query);

#endif