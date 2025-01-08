#include <postgresql/libpq-fe.h>
#include <error_handling.h>
#define DB_NAME "BankDB"

PGconn *connect_to_db()
{
    PGconn *conn = PQconnectdb("host=localhost dbname=" DB_NAME " user=thiago password=thiago port=5432 sslmode=disable");
    if (PQstatus(conn) != CONNECTION_OK)
    {
        finish_with_error(conn);
    }
    return conn;
}
