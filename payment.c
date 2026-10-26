
#include "food_system.h"

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static void print_amount(int64_t centavos)
{
	printf("PHP %" PRId64 ".%02" PRId64,
				 centavos / 100, centavos % 100);
}

void record_payment(void)
{
	int64_t orderNo;
	int choice;
	Order order;

	if (count_orders() == 0)
	{
		puts("No saved orders found.");
		return;
	}

	orderNo = read_int("Order number: ", 1, 2147483647);

	if (!read_order(orderNo, &order))
	{
		puts("Order not found or storage could not be read.");
		return;
	}

	printf("\nOrder #%" PRId64 "\n", order.orderNo);
	printf("Customer: %s\n", order.customer);
	printf("Status: %s\n", status_text(order.status));
	printf("Total: ");
	print_amount(order.totalCentavos);
	putchar('\n');

	if (order.status == STATUS_CANCELLED)
	{
		puts("Payment cannot be recorded for a cancelled order.");
		return;
	}

	puts("\nPayment method:");
	puts("1. Cash");
	puts("2. Card");
	puts("3. Bank Transfer");
	puts("4. Other");
	puts("5. Unpaid / Clear payment");

	choice = read_int("Choice: ", 1, 5);

	if (choice == 5)
	{
		order.paymentStatus = PAYMENT_UNPAID;
		order.paymentMethod = METHOD_NONE;
	}
	else
	{
		order.paymentMethod = (PaymentMethod)choice;
		order.paymentStatus = PAYMENT_UNPAID;

		if (ask_yes_no("Has payment been received? (y/n): "))
			order.paymentStatus = PAYMENT_PAID;

		if (order.paymentStatus == PAYMENT_UNPAID)
			order.paymentMethod = METHOD_NONE;
	}

	if (write_order(orderNo, &order))
	{
		printf("Payment record updated: %s.\n",
					 order.paymentStatus == PAYMENT_PAID ? "Paid" : "Unpaid");
	}
	else
	{
		puts("Could not save the payment record.");
	}
}
