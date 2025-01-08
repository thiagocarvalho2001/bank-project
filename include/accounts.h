#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <postgresql/libpq-fe.h>

void create_account(PGconn *conn);
void view_accounts(PGconn *conn);
void view_all_accounts(PGconn *conn);

#endif

