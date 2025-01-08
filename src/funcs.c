#include <stdio.h>
#include <stdlib.h>
#include "../include/error_handling.h"
#include "../include/auth.h"
#include "../include/db.h"
#include "../include/audit.h"

void deposit(PGconn *conn)
{
    int account_id;
    double amount;
    char amount_str[20], account_id_str[20];

    printf("Enter Account ID: ");
    scanf("%d", &account_id);
    snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);

    printf("Enter Amount to Deposit: ");
    scanf("%lf", &amount);
    snprintf(amount_str, sizeof(amount_str), "%.2f", amount);

    const char *paramValues[2] = {amount_str, account_id_str};
    const char *query = "UPDATE \"BankDB\".accounts set balance = balance + $1 WHERE accountid = $2";

    PGresult *res;
    res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    check_query_status(conn, query);

    const char *log_query = "INSERT INTO \"BankDB\".auditlogs (userid, event) VALUES ($1, 'Transfer')";
    const char *log_params[1] = {amount_str};
    check_query_status(conn, log_query);

    res = PQexecParams(conn, log_query, 1, NULL, log_params, NULL, NULL, 0);

    printf("Deposit successfully!\n");
    log_transf(conn, account_id, "Deposit", amount);
    PQclear(res);
}

void withdraw(PGconn *conn)
{
    int account_id;
    double amount;
    char amount_str[20], account_id_str[20];

    printf("Enter Account ID: ");
    scanf("%d", &account_id);
    snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);

    printf("Enter amount to withdraw: ");
    scanf("%lf", &amount);
    snprintf(amount_str, sizeof(amount_str), "%.2f", amount);

    const char *balance_query = "SELECT balance FROM \"BankDB\".accounts WHERE accountid = $1";
    const char *balance_param[1] = {account_id_str};
    PGresult *res = PQexecParams(conn, balance_query, 1, NULL, balance_param, NULL, NULL, 0);
    check_query_status(conn, balance_query);

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
    check_query_status(conn, update_query);

    printf("Withdrawal successful!\n");
    log_transf(conn, account_id, "Withdraw", amount);
    PQclear(res);
}

void transfer(PGconn *conn)
{
    int from_account, to_account;
    double amount;
    char from_account_str[20], amount_str[20], to_account_str[20];

    printf("Enter source account ID: ");
    scanf("%d", &from_account);
    snprintf(from_account_str, sizeof(from_account_str), "%d", from_account);

    printf("Destination account: ");
    scanf("%d", &to_account);
    snprintf(to_account_str, sizeof(to_account_str), "%d", to_account);

    printf("Amount to send: ");
    scanf("%lf", &amount);
    snprintf(amount_str, sizeof(amount_str), "%.2f", amount);

    const char *balance_query = "SELECT balance FROM \"BankDB\".accounts WHERE accountid = $1";
    const char *balance_param[1] = {from_account_str};

    PGresult *res = PQexecParams(conn, balance_query, 1, NULL, balance_param, NULL, NULL, 0);
    check_query_status(conn, balance_query);

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
    check_query_status(conn, update_query);

    const char *att_query = "UPDATE \"BankDB\".accounts set balance = balance + $1 WHERE accountid = $2";
    const char *att_param[2] = {amount_str, to_account_str};

    res = PQexecParams(conn, att_query, 2, NULL, att_param, NULL, NULL, 0);
    check_query_status(conn, att_query);

    printf("Transfer successfull!\n");
    log_transf(conn, to_account, "Transfer", amount);
    PQclear(res);
}


void generate_account_statement(PGconn *conn)
{
    int account_id;
    char start_date[20], end_date[20], account_id_str[20];

    printf("Enter your account ID: ");
    scanf("%d", &account_id);
    snprintf(account_id_str, sizeof(account_id_str), "%d", account_id);

    printf("Enter Start Date (YYYY-MM-DD): ");
    scanf("%s", start_date);

    printf("Enter End Date (YYYY-MM-DD): ");
    scanf("%s", end_date);

    const char *select_query = "SELECT * FROM \"BankDB\".transactions WHERE accountid = $1 AND timestamp BETWEEN $2 AND $3";
    const char *select_params[3] = {account_id_str, start_date, end_date};

    PGresult *res = PQexecParams(conn, select_query, 3, NULL, select_params, NULL, NULL, 0);
    check_query_status(conn, select_query);

    int nrows = PQntuples(res);

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