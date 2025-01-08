#include <stdio.h>
#include <string.h>
#include "../include/db.h"
#include "../include/accounts.h"
#include "../include/auth.h"
#include "../include/customers.h"
#include "../include/funcs.h"

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