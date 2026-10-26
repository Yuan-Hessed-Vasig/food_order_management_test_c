
#include "food_system.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_TALLY 200

typedef struct
{
	char name[50];
	int quantity;
} ItemTally;

static void print_money(int64_t centavos)
{
	printf("PHP %" PRId64 ".%02" PRId64,
				 centavos / 100, centavos % 100);
}

static int valid_date(const char *date)
{
	if (strlen(date) != 10 ||
			date[4] != '-' || date[7] != '-')
		return 0;

	for (int i = 0; i < 10; i++)
	{
		if (i == 4 || i == 7)
			continue;
		if (date[i] < '0' || date[i] > '9')
			return 0;
	}

	int year = (date[0] - '0') * 1000 +
						 (date[1] - '0') * 100 +
						 (date[2] - '0') * 10 +
						 (date[3] - '0');

	int month = (date[5] - '0') * 10 + (date[6] - '0');
	int day = (date[8] - '0') * 10 + (date[9] - '0');

	if (year < 1 || month < 1 || month > 12)
		return 0;

	static const int daysInMonth[] = {
			31, 28, 31, 30, 31, 30,
			31, 31, 30, 31, 30, 31};

	int maxDay = daysInMonth[month - 1];

	if (month == 2 &&
			(year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)))
		maxDay = 29;

	return day >= 1 && day <= maxDay;
}

static void add_item_tally(
		ItemTally tally[],
		int *count,
		const char *name,
		int quantity)
{
	for (int i = 0; i < *count; i++)
	{
		if (strcmp(tally[i].name, name) == 0)
		{
			tally[i].quantity += quantity;
			return;
		}
	}

	if (*count >= MAX_TALLY)
		return;

	snprintf(tally[*count].name, sizeof tally[*count].name,
					 "%s", name);
	tally[*count].quantity = quantity;
	(*count)++;
}

void daily_sales_order_report(void)
{
	char date[32];
	Order order;
	ItemTally tally[MAX_TALLY] = {0};

	int totalOrders = count_orders();
	int ordersFound = 0;
	int paidOrders = 0;
	int unpaidOrders = 0;
	int cancelledOrders = 0;
	int itemCount = 0;

	int64_t grossSales = 0;
	int64_t collected = 0;
	int64_t unpaidAmount = 0;

	read_line("Report date (YYYY-MM-DD): ", date, sizeof date);

	if (!valid_date(date))
	{
		puts("Invalid date. Use YYYY-MM-DD.");
		return;
	}

	for (int64_t no = 1; no <= totalOrders; no++)
	{
		if (!read_order(no, &order))
			continue;

		if (strcmp(order.date, date) != 0)
			continue;

		ordersFound++;

		if (order.status == STATUS_CANCELLED)
		{
			cancelledOrders++;
			continue;
		}

		grossSales += order.totalCentavos;

		if (order.paymentStatus == PAYMENT_PAID)
		{
			paidOrders++;
			collected += order.totalCentavos;
		}
		else
		{
			unpaidOrders++;
			unpaidAmount += order.totalCentavos;
		}

		for (int i = 0; i < order.itemCount; i++)
		{
			add_item_tally(
					tally,
					&itemCount,
					order.items[i].name,
					order.items[i].quantity);
		}
	}

	printf("\n========== DAILY REPORT ==========\n");
	printf("Date: %s\n", date);
	printf("Orders found: %d\n", ordersFound);
	printf("Paid orders: %d\n", paidOrders);
	printf("Unpaid orders: %d\n", unpaidOrders);
	printf("Cancelled orders: %d\n", cancelledOrders);

	printf("\nSales from non-cancelled orders: ");
	print_money(grossSales);

	printf("\nPayments collected: ");
	print_money(collected);

	printf("\nOutstanding unpaid amount: ");
	print_money(unpaidAmount);

	puts("\n\n========== ITEMS SOLD ==========");

	if (itemCount == 0)
	{
		puts("No items sold on this date.");
	}
	else
	{
		for (int i = 0; i < itemCount; i++)
			printf("%-30s %d\n",
						 tally[i].name, tally[i].quantity);
	}

	puts("================================");
	press_any_key();
}
