#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <postgresql/libpq-fe.h>

#define DB_NAME "BankDB"

void finish_with_error(PGconn *conn){
    fprintf(stderr, "%s\n", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

PGconn* connect_to_db(){
    PGconn *conn = PQconnectdb("host=localhost dbname="DB_NAME" user=thiago password=thiago");

    if (PQstatus(conn) != CONNECTION_OK) {
        finish_with_error(conn);
    }
    return conn;
}

void add_customer(PGconn *conn){
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
    
    res = PQexecParams(conn, query, 4, NULL, (const char *[]) {name, address, phone, email}, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        finish_with_error(conn);
    }

    printf("Customer added!\n");
    PQclear(res);
}

void main_menu(){
    PGconn *conn = connect_to_db();
    int choice;

    do{
        printf("\n----- BMS -----\n ");
        printf("1. To add customer\n");
        printf("2. Exit \n");
        printf("Choose one: ");
        scanf("%d", &choice);

        switch(choice){
            case 1:
                add_customer(conn);
                break;
            case 2:
                printf("Logout...\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }while(choice != 2);

    PQfinish(conn);
}

int main(){
    main_menu();
    return 0;
}