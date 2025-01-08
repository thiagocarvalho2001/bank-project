#ifndef CUSTOMERS_H
#define CUSTOMERS_H

#include <postgresql/libpq-fe.h>

void add_customer(PGconn *conn);
void view_customer_details(PGconn *conn);

#endif


