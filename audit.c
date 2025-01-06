#include <stdio.h>
#include <postgresql/libpq-fe.h>
#include "auth.h"



void log_event(PGconn *conn, int user_id, const char *event)
{   
    char user_id_str[30];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    const char *add_log = "INSERT INTO \"BankDB\".auditlogs (userid, event) VALUES ($1, $2)";
    const char *param_values[2] = {user_id_str, event};

    PGresult *res = PQexecParams(conn, add_log, 2, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
}

void log_transf(PGconn *conn, int accountid, char *transactiontype, int amount){
    char accountid_str[30];
    char amount_str[30];

    snprintf(amount_str, sizeof(amount_str), "%d", amount);
    snprintf(accountid_str, sizeof(accountid_str), "%d", accountid);

    const char *add_transf = "INSERT INTO \"BankDB\".transactions (accountid, transactiontype, amount) VALUES ($1, $2, $3)";
    const char *param_values[3] = {accountid_str, transactiontype, amount_str};

    PGresult *res = PQexecParams(conn, add_transf, 3, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
}

