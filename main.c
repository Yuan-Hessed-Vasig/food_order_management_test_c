
#include "food_system.h"

#include <stdio.h>

int main(void)
{
	Food foods[MAX_FOODS];
	int foodCount = load_menu(foods);
	int choice;

	do
	{
		puts("\n====== FOOD ORDER MANAGEMENT SYSTEM ======");
		puts("1. View Menu");
		puts("2. Manage Menu");
		puts("3. Create Order");
		puts("4. Update Order Status");
		puts("5. Record Payment");
		puts("6. Daily Order/Sales Report");
		puts("7. Exit");

		choice = read_int("Enter choice: ", 1, 7);

		switch (choice)
		{
		case 1:
			view_menu(foods, foodCount);
			break;

		case 2:
			manage_menu(foods, &foodCount);
			break;

		case 3:
			create_order(foods, foodCount);
			break;

		case 4:
			update_order_status();
			break;

		case 5:
			record_payment();
			break;

		case 6:
			daily_sales_order_report();
			break;

		case 7:
			puts("Exiting Food Order Management System.");
			break;
		}
	} while (choice != 7);

	return 0;
}
