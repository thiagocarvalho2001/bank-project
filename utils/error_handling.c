#include "../include/db.h"
#include <stdlib.h>
#include <stdio.h>


void finish_with_error(PGconn *conn)
{
    fprintf(stderr, "%s\n", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

int check_query_status(PGconn *conn, const char *query){
    PGresult *res = PQexec(conn, query);

    if(res == NULL)
    {
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    if(PQcmdTuples(res) != NULL){
        PQclear(res);
        return 0;
    }


    if(PQresultStatus(res) != PGRES_COMMAND_OK || PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }

    fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    return 1;
}