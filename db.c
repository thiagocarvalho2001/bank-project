#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <postgresql/libpq-fe.h>
#define DB_NAME "BankDB"

void finish_with_error(PGconn *conn)
{
    fprintf(stderr, "%s\n", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

PGconn *connect_to_db()
{
    PGconn *conn = PQconnectdb("host=localhost dbname=" DB_NAME " user=thiago password=thiago");
    if (PQstatus(conn) != CONNECTION_OK)
    {
        finish_with_error(conn);
    }
    return conn;
}

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
    PGresult *res;
    res = PQexecParams(conn, query, 4, NULL, (const char *[]){name, address, phone, email}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        finish_with_error(conn);
    }
    printf("Customer added!\n");
    PQclear(res);
}

void create_account(PGconn *conn) {
    int customer_id;
    int accountid;
    char account_type[10];
    double initial_balance;
    char initial_balance_str[20];
    char customer_id_str[20];
    char accountid_str[20];

    printf("Enter customer ID: ");
    scanf("%d", &customer_id);
    sprintf(customer_id_str, "%d", customer_id);

    printf("Enter account ID: ");
    scanf("%d", &accountid);
    sprintf(accountid_str, "%d", accountid);

    printf("Enter Account Type (Savings/Current): ");
    scanf("%s", account_type);

    printf("Enter initial balance: ");
    scanf("%lf", &initial_balance);
    sprintf(initial_balance_str, "%.2f", initial_balance);

    const char *query = "INSERT INTO \"BankDB\".accounts (accountid, customerid, accounttype, balance) VALUES ($1, $2, $3, $4)";

    PGresult *res;

    const char *values[] = {accountid_str, customer_id_str, account_type, initial_balance_str};

    res = PQexecParams(conn, query, 4, NULL, values, NULL, NULL, 0); 

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        finish_with_error(conn);
    }
    printf("Account created!\n");
    PQclear(res);
}

void view_accounts(PGconn *conn)
{
    int choice, customer_id, account_id;
    char customer_id_str[12];
    char account_id_str[12];
    char query[256];

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

        PGresult *res;

        res = PQexecParams(conn, query, 1, NULL, (const char *[]){(char *)&customer_id_str}, NULL, NULL, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            return;
        }

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
    }else{
        printf("Enter account ID: ");
        scanf("%d", &account_id);

        snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);
        const char *query = "SELECT * FROM \"BankDB\".accounts WHERE accountid = $1";

        PGresult *res;

        res = PQexecParams(conn, query, 1, NULL, (const char *[]){(char *)&account_id_str}, NULL, NULL, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            return;
        }

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

void main_menu()
{
    PGconn *conn = connect_to_db();
    int choice;

    do
    {
        printf("\n -- Bank Management System -- \n");
        printf("1. Add customer\n");
        printf("2. Create account\n");
        printf("3. View Accounts\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            add_customer(conn);
            break;
        case 2:
            create_account(conn);
            break;
        case 3:
            view_accounts(conn);
            break;
        case 4:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid. Try again.\n");
        }
    } while (choice != 4);
}

int main()
{
    main_menu();
    return 0;
}