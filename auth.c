#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <postgresql/libpq-fe.h>
#include <openssl/sha.h>
#include "auth.h"
#include "audit.h"

#define DB_NAME "BankDB"

void hash_password(const char *password, char *hashed_password){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password, strlen(password), hash);

    for(int i=0; i<SHA256_DIGEST_LENGTH; i++){
        sprintf(hashed_password + i*2, "%02x", hash[i]);
    }
    hashed_password[64] = '\0';
}

int authenticate_user(PGconn *conn, const char *username, const char *password, char *role)
{
    char hashed_password[65];
    const char *hash_query = "SELECT userid, passwordhash, role, failedattempts, locked FROM \"BankDB\".users WHERE username = $1";
    const char *param_value[1] = {username};

    PGresult *res = PQexecParams(conn, hash_query, 1, NULL, param_value, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);
    if (nrows == 0)
    {
        printf("Invalid username or password.\n");
        PQclear(res);
        return 0;
    }

    int user_id = atoi(PQgetvalue(res, 0, 0));
    hash_password(password, hashed_password);

    if (strcmp(hashed_password, PQgetvalue(res, 0, 1)) != 0)
    {
        printf("Invalid username or password.\n");
        log_event(conn, user_id, "Failed login");

        const char *att_query = "UPDATE \"BankDB\".users SET failedattempts = failedattempts + 1 WHERE userid = $1";
        const char *att_param[1] = {PQgetvalue(res, 0, 0)};
        
        PQexecParams(conn, att_query, 1, NULL, att_param, NULL, NULL, 0);

        if (atoi(PQgetvalue(res, 0, 3)) >= 3)
        {
            const char *lock_query = "UPDATE \"BankDB\".users SET locked = 1 WHERE userid = $1";
            PQexecParams(conn, lock_query, 1, NULL, att_param, NULL, NULL, 0);
        }

        PQclear(res);
        return 0;
    }

    if (atoi(PQgetvalue(res, 0, 4)) == 1)
    {
        printf("Account is locked due to too many failed login attempts.\n");
        PQclear(res);
        return 0;
    }

    const char *reset_query = "UPDATE \"BankDB\".users SET failedattempts = 0 WHERE userid = $1";
    const char *reset_param[1] = {PQgetvalue(res, 0, 0)};
    PQexecParams(conn, reset_query, 1, NULL, reset_param, NULL, NULL, 0);

    strcpy(role, PQgetvalue(res, 0, 2));  
    log_event(conn, user_id, "Successful login");

    PQclear(res);
    return 1;
}



void create_user(PGconn *conn, const char *username, const char *password, const char *role)
{
    char query[512], hashed_password[65];
    hash_password(password, hashed_password);

    const char *create_query = "INSERT INTO \"BankDB\".users (username, passwordhash, role) VALUES ($1, $2, $3)";
    const char *params_values[3] = {username, hashed_password, role};

    PGresult *res = PQexecParams(conn, create_query, 3, NULL, params_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        finish_with_error(conn);
    }

    printf("User: '%s' created. \n", username);
    PQclear(res);
}
