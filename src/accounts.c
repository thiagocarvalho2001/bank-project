#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/error_handling.h"
#include "../include/db.h"
#include "../include/audit.h"

void create_account(PGconn *conn)
{
    int customer_id, accountid;
    char account_type[10], initial_balance_str[20], customer_id_str[20], accountid_str[20];
    double initial_balance;

    printf("Enter customer ID: ");
    scanf("%d", &customer_id);
    snprintf(customer_id_str, sizeof(customer_id_str), "%d", customer_id);

    printf("Enter account ID: ");
    scanf("%d", &accountid);
    snprintf(accountid_str, sizeof(accountid_str), "%d", accountid);

    printf("Enter Account Type (Savings/Current): ");
    scanf("%s", account_type);

    printf("Enter initial balance: ");
    scanf("%lf", &initial_balance);
    snprintf(initial_balance_str, sizeof(initial_balance_str), "%.2f", initial_balance);

    const char *query = "INSERT INTO \"BankDB\".accounts (accountid, customerid, accounttype, balance) VALUES ($1, $2, $3, $4)";
    const char *values[] = {accountid_str, customer_id_str, account_type, initial_balance_str};
    PGresult *res = PQexecParams(conn, query, 4, NULL, values, NULL, NULL, 0);
    check_query_status(conn, query);
    log_event(conn, accountid, "Account created");
    printf("Account created!\n");
    PQclear(res);
}

void view_accounts(PGconn *conn)
{
    int choice, customer_id, account_id;
    char customer_id_str[12], account_id_str[12];

    printf("View accounts by: \n");
    printf("1. Customer ID\n");
    printf("2. Account ID\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        printf("Enter customer ID: ");
        scanf("%d", &customer_id);
        snprintf(customer_id_str, sizeof(customer_id_str), "%d", customer_id);


        const char *query = "SELECT * FROM \"BankDB\".accounts WHERE customerid = $1";
        PGresult *res = PQexecParams(conn, query, 1, NULL, (const char *[]){(char *)&customer_id_str}, NULL, NULL, 0);
        check_query_status(conn, query);
        int nrows = PQntuples(res);
        int ncols = PQnfields(res);

        printf("\nAccount details: \n");
        printf("AccountID | CustomerID | AccountType | Balance | CreatedAt \n");

        for (int i = 0; i < nrows; i++)
        {
            for (int j = 0; j < ncols; j++)
            {
                printf("%s", PQgetvalue(res, i, j));
                if (j < ncols - 1)
                {
                    printf(" | ");
                }
            }
            printf("\n");
        }
        PQclear(res);
    }
    else
    {
        printf("Enter account ID: ");
        scanf("%d", &account_id);
        snprintf(customer_id_str, sizeof(customer_id_str), "%d", customer_id);

        const char *query = "SELECT * FROM \"BankDB\".accounts WHERE accountid = $1";
        PGresult *res = PQexecParams(conn, query, 1, NULL, (const char *[]){(char *)&account_id_str}, NULL, NULL, 0);
        check_query_status(conn, query);
        int nrows = PQntuples(res);
        int ncols = PQnfields(res);

        printf("\nAccount details: \n");
        printf("AccountID | CustomerID | AccountType | Balance | CreatedAt \n");

        for (int i = 0; i < nrows; i++)
        {
            for (int j = 0; j < ncols; j++)
            {
                printf("%s", PQgetvalue(res, i, j));
                if (j < ncols - 1)
                {
                    printf(" | ");
                }
            }
            printf("\n");
        }
        PQclear(res);
    }
}

void view_all_accounts(PGconn *conn)
{
    const char *query = "SELECT * FROM \"BankDB\".accounts";
    PGresult *res = PQexecParams(conn, query, 0, NULL, NULL, NULL, NULL, 0);
    check_query_status(conn, query);
    int nrows = PQntuples(res);

    if (nrows == 0)
    {
        printf("No accounts found.\n");
        PQclear(res);
        return;
    }

    printf("AccountID | CustomerID | Account Type | Balance \n");

    for (int i = 0; i < nrows; i++)
    {
        double balance_acc = atof(PQgetvalue(res, i, 3));
        printf("%s | %s | %s | %.2f \n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               PQgetvalue(res, i, 2),
               balance_acc);
    }
    PQclear(res);
}