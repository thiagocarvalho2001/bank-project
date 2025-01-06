#ifndef AUTH_H
#define AUTH_H

#include <postgresql/libpq-fe.h>

void hash_password(const char *password, char *hashed_password);
int authenticate_user(PGconn *conn, const char *username, const char *password, char *role);
void create_user(PGconn *conn, const char *username, const char *password, const char *role);
void finish_with_error(PGconn *conn);
#endif 