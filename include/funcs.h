#ifndef FUNCS_H
#define FUNCS_H

#include <postgresql/libpq-fe.h>

void deposit(PGconn *conn);
void withdraw(PGconn *conn);
void transfer(PGconn *conn);
void generate_account_statement(PGconn *conn);

#endif
