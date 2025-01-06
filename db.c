#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <postgresql/libpq-fe.h>
#include "auth.h"
#include "audit.h"
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

void create_account(PGconn *conn)
{
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
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
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
    }
    else
    {
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

void deposit(PGconn *conn)
{
    int account_id;
    double amount;
    char amount_str[20];
    char account_id_str[20];

    printf("Enter Account ID: ");
    scanf("%d", &account_id);

    printf("Enter Amount to Deposit: ");
    scanf("%lf", &amount);

    snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);
    snprintf(amount_str, sizeof(amount_str), "%.2f", amount);

    const char *paramValues[2] = {amount_str, account_id_str};
    char query[256];
    snprintf(query, sizeof(query),
             "UPDATE \"BankDB\".accounts set balance = balance + $1 WHERE accountid = $2");

    PGresult *res;
    res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        finish_with_error(conn);
    }

    const char *log_query = "INSERT INTO \"BankDB\".auditlogs (userid, event) VALUES ($1, 'Transfer')";
    const char *log_params[1] = {amount_str};

    res = PQexecParams(conn, query, 1, NULL, log_params, NULL, NULL, 0);

    printf("Deposit successfully!\n");
    log_transf(conn, account_id, "Deposit", amount);
    PQclear(res);
}

void withdraw(PGconn *conn)
{
    int account_id;
    double amount;
    char amount_str[20];
    char account_id_str[20];

    printf("Enter Account ID: ");
    scanf("%d", &account_id);

    printf("Enter amount to withdraw: ");
    scanf("%lf", &amount);

    snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);
    snprintf(amount_str, sizeof(amount_str), "%.2f", amount);

    const char *balance_query = "SELECT balance FROM \"BankDB\".accounts WHERE accountid = $1";
    const char *balance_param[1] = {account_id_str};

    PGresult *res = PQexecParams(conn, balance_query, 1, NULL, balance_param, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);
    if (nrows == 0)
    {
        printf("Account not found.\n");
        PQclear(res);
        return;
    }

    const char *balance_str = PQgetvalue(res, 0, 0);
    double current_balance = atof(balance_str);

    if (current_balance < amount)
    {
        printf("Insufficient funds. Current balance: %.2f\n", current_balance);
        PQclear(res);
        return;
    }

    const char *update_query = "UPDATE \"BankDB\".accounts SET balance = balance - $1 WHERE accountid = $2";
    const char *update_param[2] = {amount_str, account_id_str};

    res = PQexecParams(conn, update_query, 2, NULL, update_param, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        finish_with_error(conn);
    }

    printf("Withdrawal successful!\n");
    log_transf(conn, account_id, "Withdraw", amount);
    PQclear(res);
}

void transfer(PGconn *conn)
{
    int from_account, to_account;
    double amount;
    char from_account_str[20];
    char to_account_str[20];
    char amount_str[20];

    printf("Enter source account ID: ");
    scanf("%d", &from_account);

    printf("Destination account: ");
    scanf("%d", &to_account);

    printf("Amount to send: ");
    scanf("%lf", &amount);

    snprintf(from_account_str, sizeof(from_account_str), "%d", from_account);
    snprintf(to_account_str, sizeof(to_account_str), "%d", to_account);
    snprintf(amount_str, sizeof(amount_str), "%.2f", amount);

    const char *balance_query = "SELECT balance FROM \"BankDB\".accounts WHERE accountid = $1";
    const char *balance_param[1] = {from_account_str};

    PGresult *res = PQexecParams(conn, balance_query, 1, NULL, balance_param, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error fetching balance: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);
    if (nrows == 0)
    {
        printf("Source account not found.\n");
        PQclear(res);
        return;
    }

    const char *balance_str = PQgetvalue(res, 0, 0);
    double current_balance = atof(balance_str);

    if (current_balance < amount)
    {
        printf("Insufficient funds. Current balance: %.2f\n", current_balance);
        PQclear(res);
        return;
    }

    const char *update_query = "UPDATE \"BankDB\".accounts SET balance = balance - $1 where accountid = $2";
    const char *update_param[2] = {amount_str, from_account_str};

    res = PQexecParams(conn, update_query, 2, NULL, update_param, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        finish_with_error(conn);
    }

    const char *att_query = "UPDATE \"BankDB\".accounts set balance = balance + $1 WHERE accountid = $2";
    const char *att_param[2] = {amount_str, to_account_str};

    res = PQexecParams(conn, att_query, 2, NULL, att_param, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);  
    }
    printf("Transfer successfull!\n");
    log_transf(conn, to_account, "Transfer", amount);
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

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error 1: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    if (res == NULL)
    {
        finish_with_error(conn);
    }

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

    if (PQresultStatus(res) != PGRES_TUPLES_OK || res == NULL)
    {
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    printf("Account details: \n");
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

void view_all_accounts(PGconn *conn)
{
    const char *select_query = "SELECT * FROM \"BankDB\".accounts";
    PGresult *res = PQexecParams(conn, select_query, 0, NULL, NULL, NULL, NULL, 0);

    if (res == NULL)
    {
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);
    if (nrows == 0)
    {
        printf("No accounts found.\n");
        PQclear(res);
        return;
    }

    printf("All accounts: \n");
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

void generate_account_statement(PGconn *conn)
{
    int account_id;
    char start_date[20], end_date[20], account_id_str[20];

    printf("Enter your account ID");
    scanf("%d", &account_id);

    printf("Enter Start Date (YYYY-MM-DD): ");
    scanf("%s", start_date);

    printf("Enter End Date (YYYY-MM-DD): ");
    scanf("%s", end_date);

    snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);

    const char *select_query = "SELECT * FROM \"BankDB\".transactions WHERE accountid = $1 AND timestamp BETWEEN $2 AND $3";
    const char *select_params[3] = {account_id_str, start_date, end_date};

    PGresult *res = PQexecParams(conn, select_query, 3, NULL, select_params, NULL, NULL, 0);
    if (res == NULL)
    {
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);

    printf("\nAccount Statement: \n");
    printf("Transaction ID | Transaction Type | Amount | Timestamp \n");
    if (nrows == 0)
    {
        printf("No transactions found.\n");
        PQclear(res);
        return;
    }else{
        for (int i = 0; i < nrows; i++)
        {
            double amount = atof(PQgetvalue(res, i, 2));
            printf("%s | %s | %.2f | %s \n",
            PQgetvalue(res, i, 0),
            PQgetvalue(res, i, 1),
            amount,
            PQgetvalue(res, i, 3));
        }
    }
    PQclear(res);
}

void main_menu()
{
    PGconn *conn = connect_to_db();
    char username[50], password[50], role[10];
    int authenticated = 0;

    while(!authenticated){
        printf("Login: \n");
        printf("Username: ");
        scanf("%s", username);
        printf("Password: ");
        scanf("%s", password);

        authenticated = authenticate_user(conn, username, password, role);
    }

    int choice;

    do
    {
        printf("\n -- Bank Management System -- \n");
        printf("1. Add customer\n");
        printf("2. Create account\n");
        printf("3. View Accounts\n");
        printf("4. Deposit Money\n");
        printf("5. Withdraw Money\n");
        printf("6. Transfer Money\n");
        printf("7. View Customer Details\n");
        printf("8. View All Accounts\n");
        printf("9. Generate Account Statement\n");

        if(strcmp(role, "Admin") == 0){
            printf("10. Create user\n");
        }

        printf("11. Exit\n");
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
            deposit(conn);
            break;
        case 5:
            withdraw(conn);
            break;
        case 6:
            transfer(conn);
            break;
        case 7:
            view_customer_details(conn);
            break;
        case 8:
            view_all_accounts(conn);
            break;
        case 9:
            generate_account_statement(conn);
            break;
        case 10:
            if(strcmp(role, "Admin") == 0){
                char new_username[50], new_password[50], new_role[10];
                printf("Enter new username: ");
                scanf("%s", new_username);
                printf("Enter new password: ");
                scanf("%s", new_password);
                printf("Enter new role: ");
                scanf("%s", new_role);
                create_user(conn, new_username, new_password, new_role);
            }else{
                printf("You are not an admin.\n");
            };
            break;
        case 11:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid. Try again.\n");
        }
    } while (choice != 11);
}

int main()
{
    main_menu();
    return 0;
}