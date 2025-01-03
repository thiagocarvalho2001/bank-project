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

PGconn *connect_to_db(){
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

    printf("Deposit successfully!\n");
    PQclear(res);
}

void withdraw(PGconn *conn) {
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

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);
    if (nrows == 0) {
        printf("Account not found.\n");
        PQclear(res);
        return;
    }

    const char *balance_str = PQgetvalue(res, 0, 0);
    double current_balance = atof(balance_str);

    if (current_balance < amount) {
        printf("Insufficient funds. Current balance: %.2f\n", current_balance);
        PQclear(res);
        return;
    }

    const char *update_query = "UPDATE \"BankDB\".accounts SET balance = balance - $1 WHERE accountid = $2";
    const char *update_param[2] = {amount_str, account_id_str};

    res = PQexecParams(conn, update_query, 2, NULL, update_param, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        finish_with_error(conn);
    }

    printf("Withdrawal successful!\n");
    PQclear(res);
}

void transfer(PGconn *conn){
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
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error fetching balance: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    int nrows = PQntuples(res);
    if(nrows == 0){
        printf("Source account not found.\n");
        PQclear(res);
        return;
    }

    const char *balance_str = PQgetvalue(res, 0, 0);
    double current_balance = atof(balance_str);

    if(current_balance < amount){
        printf("Insufficient funds. Current balance: %.2f\n", current_balance);
        PQclear(res);
        return;
    }

    const char *update_query = "UPDATE \"BankDB\".accounts SET balance = balance - $1 where accountid = $2";
    const char *update_param[2] = {amount_str, from_account_str};

    res = PQexecParams(conn, update_query, 2, NULL, update_param, NULL, NULL, 0);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        finish_with_error(conn);
    }

    const char *att_query = "UPDATE \"BankDB\".accounts set balance = balance + $1 WHERE accountid = $2";

    const char *att_param[2] = {amount_str, to_account_str};
    res = PQexecParams(conn, att_query, 2, NULL, att_param, NULL, NULL, 0);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn));
        finish_with_error(conn);
    }

    printf("Transfer successfull!\n");

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
        printf("4. Deposit Money\n");
        printf("5. Withdraw Money\n");
        printf("6. Transfer Money\n");
        printf("7. Exit\n");
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
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid. Try again.\n");
        }
    } while (choice != 7);
}

int main()
{
    main_menu();
    return 0;
}