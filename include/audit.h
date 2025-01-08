#ifndef AUDIT_H
#define AUDIT_H

#include <postgresql/libpq-fe.h>

void log_event(PGconn *conn, int user_id, const char *event);
void log_transf(PGconn *conn, int accountid, const char *transactiontype, int amount);

#endif 