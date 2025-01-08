#include "../include/db.h"
#include "../include/audit.h"
#include "../include/error_handling.h"
#include <stdio.h>

void add_customer(PGconn *conn)
{
    char name[100], address[255], phone[20], email[100];

    printf("Enter Customer Name: ");
    scanf(" %[^\n]", name);
    printf("Enter Address: ");
    scanf(" %[^\n]", address);
    printf("Enter Phone number: ");
    scanf("%s", phone);
    printf("Enter email: ");
    scanf("%s", email);

    const char *query = "INSERT INTO \"BankDB\".customers (name, address, phonenumber, email) VALUES ($1, $2, $3, $4)";
    PGresult *res = PQexecParams(conn, query, 4, NULL, (const char *[]){name, address, phone, email}, NULL, NULL, 0);
    check_query_status(conn, query);

    printf("Customer added!\n");
    PQclear(res);
}

void view_customer_details(PGconn *conn)
{
    int customer_id;
    char customer_id_str[20];

    printf("Enter Customer ID: ");
    scanf("%d", &customer_id);
    snprintf(customer_id_str, sizeof(customer_id_str), "%d", customer_id);

    const char *view_query = "SELECT * FROM \"BankDB\".customers WHERE customerid = $1";
    const char *view_params[1] = {customer_id_str};
    PGresult *res = PQexecParams(conn, view_query, 1, NULL, view_params, NULL, NULL, 0);
    check_query_status(conn, view_query);

    int nrows = PQntuples(res);
    if (nrows == 0)
    {
        printf("Customer not found.\n");
        PQclear(res);
        return;
    }

    printf("Customer details: \n");
    for (int i = 0; i < nrows; i++)
    {
        printf("Customer ID: %s\n", PQgetvalue(res, i, 0));
        printf("Name: %s\n", PQgetvalue(res, i, 1));
        printf("Address: %s\n", PQgetvalue(res, i, 2));
        printf("Phone Number: %s\n", PQgetvalue(res, i, 3));
        printf("Email: %s\n", PQgetvalue(res, i, 4));
        printf("Created At: %s'\n", PQgetvalue(res, i, 5));
    }
    PQclear(res);

    const char *select_query = "SELECT accountid, accounttype, balance FROM \"BankDB\".accounts WHERE customerid = $1";
    const char *select_params[1] = {customer_id_str};

    res = PQexecParams(conn, select_query, 1, NULL, select_params, NULL, NULL, 0);
    check_query_status(conn, select_query);

    printf("AccountID | Account Type | Balance \n");
    for (int i = 0; i < nrows; i++)
    {
        printf("%s | %s | %s \n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               PQgetvalue(res, i, 2));
    }
    PQclear(res);
}
