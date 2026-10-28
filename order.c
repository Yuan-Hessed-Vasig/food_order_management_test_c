
#include "food_system.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#define STORE_VERSION 1
#define STORE_MAGIC "FOODORD2"

static int write_value(FILE *f, const void *value, size_t size)
{
	return fwrite(value, size, 1, f) == 1;
}

static int read_value(FILE *f, void *value, size_t size)
{
	return fread(value, size, 1, f) == 1;
}

/*
 * Records are stored field by field, not by writing
 * the entire Order structure. Every record has a fixed layout.
 */
static int write_record(FILE *f, const Order *o)
{
	int32_t itemCount = o->itemCount;
	int32_t status = o->status;
	int32_t paymentStatus = o->paymentStatus;
	int32_t paymentMethod = o->paymentMethod;

	if (itemCount < 0 || itemCount > MAX_ITEMS ||
			status < STATUS_PENDING || status >= STATUS_COUNT ||
			(paymentStatus != PAYMENT_UNPAID &&
			 paymentStatus != PAYMENT_PAID) ||
			paymentMethod < METHOD_NONE ||
			paymentMethod > METHOD_OTHER)
		return 0;

	if (!write_value(f, &o->orderNo, sizeof o->orderNo) ||
			!write_value(f, o->customer, sizeof o->customer) ||
			!write_value(f, o->date, sizeof o->date) ||
			!write_value(f, &itemCount, sizeof itemCount))
		return 0;

	for (int i = 0; i < MAX_ITEMS; i++)
	{
		const OrderItem *item = &o->items[i];

		if (!write_value(f, &item->foodId, sizeof item->foodId) ||
				!write_value(f, item->name, sizeof item->name) ||
				!write_value(f, &item->unitPriceCentavos,
										 sizeof item->unitPriceCentavos) ||
				!write_value(f, &item->quantity, sizeof item->quantity) ||
				!write_value(f, &item->subtotalCentavos,
										 sizeof item->subtotalCentavos))
			return 0;
	}

	return write_value(f, &o->totalCentavos, sizeof o->totalCentavos) &&
				 write_value(f, &status, sizeof status) &&
				 write_value(f, &paymentStatus, sizeof paymentStatus) &&
				 write_value(f, &paymentMethod, sizeof paymentMethod);
}

static int read_record(FILE *f, Order *o)
{
	int32_t itemCount, status, paymentStatus, paymentMethod;

	memset(o, 0, sizeof *o);

	if (!read_value(f, &o->orderNo, sizeof o->orderNo) ||
			!read_value(f, o->customer, sizeof o->customer) ||
			!read_value(f, o->date, sizeof o->date) ||
			!read_value(f, &itemCount, sizeof itemCount))
		return 0;

	o->customer[sizeof o->customer - 1] = '\0';
	o->date[sizeof o->date - 1] = '\0';

	for (int i = 0; i < MAX_ITEMS; i++)
	{
		OrderItem *item = &o->items[i];

		if (!read_value(f, &item->foodId, sizeof item->foodId) ||
				!read_value(f, item->name, sizeof item->name) ||
				!read_value(f, &item->unitPriceCentavos,
										sizeof item->unitPriceCentavos) ||
				!read_value(f, &item->quantity, sizeof item->quantity) ||
				!read_value(f, &item->subtotalCentavos,
										sizeof item->subtotalCentavos))
			return 0;

		item->name[sizeof item->name - 1] = '\0';
	}

	if (!read_value(f, &o->totalCentavos, sizeof o->totalCentavos) ||
			!read_value(f, &status, sizeof status) ||
			!read_value(f, &paymentStatus, sizeof paymentStatus) ||
			!read_value(f, &paymentMethod, sizeof paymentMethod))
		return 0;

	if (itemCount < 0 || itemCount > MAX_ITEMS ||
			status < STATUS_PENDING || status >= STATUS_COUNT ||
			(paymentStatus != PAYMENT_UNPAID &&
			 paymentStatus != PAYMENT_PAID) ||
			paymentMethod < METHOD_NONE ||
			paymentMethod > METHOD_OTHER)
		return 0;

	o->itemCount = itemCount;
	o->status = (OrderStatus)status;
	o->paymentStatus = (PaymentStatus)paymentStatus;
	o->paymentMethod = (PaymentMethod)paymentMethod;

	return 1;
}

static int valid_store(FILE *f)
{
	char magic[8];
	uint32_t version;

	rewind(f);

	return read_value(f, magic, sizeof magic) &&
				 memcmp(magic, STORE_MAGIC, sizeof magic) == 0 &&
				 read_value(f, &version, sizeof version) &&
				 version == STORE_VERSION;
}

static int initialize_store(FILE *f)
{
	const char magic[8] = STORE_MAGIC;
	const uint32_t version = STORE_VERSION;

	rewind(f);

	return write_value(f, magic, sizeof magic) &&
				 write_value(f, &version, sizeof version) &&
				 fflush(f) == 0;
}

int count_orders(void)
{
	FILE *f = fopen(ORDER_FILE, "rb");
	Order o;
	int count = 0;

	if (!f)
		return 0;

	if (!valid_store(f))
	{
		fclose(f);
		return 0;
	}

	while (read_record(f, &o))
		count++;

	fclose(f);
	return count;
}

int read_order(int64_t orderNo, Order *o)
{
	FILE *f = fopen(ORDER_FILE, "rb");
	Order current;

	if (!f)
		return 0;

	if (!valid_store(f))
	{
		fclose(f);
		return 0;
	}

	while (read_record(f, &current))
	{
		if (current.orderNo == orderNo)
		{
			*o = current;
			fclose(f);
			return 1;
		}
	}

	fclose(f);
	return 0;
}

int write_order(int64_t orderNo, const Order *o)
{
	FILE *f = fopen(ORDER_FILE, "r+b");
	Order current;

	if (!f)
		return 0;

	if (!valid_store(f))
	{
		fclose(f);
		return 0;
	}

	for (;;)
	{
		long position = ftell(f);

		if (position < 0 || !read_record(f, &current))
			break;

		if (current.orderNo == orderNo)
		{
			if (fseek(f, position, SEEK_SET) != 0)
			{
				fclose(f);
				return 0;
			}

			int success = write_record(f, o) && fflush(f) == 0;
			fclose(f);
			return success;
		}
	}

	fclose(f);
	return 0;
}

int append_order(const Order *o)
{
	FILE *f = fopen(ORDER_FILE, "r+b");

	if (!f)
	{
		f = fopen(ORDER_FILE, "w+b");

		if (!f)
			return 0;

		if (!initialize_store(f))
		{
			fclose(f);
			return 0;
		}
	}
	else if (!valid_store(f))
	{
		fclose(f);
		return 0;
	}

	if (fseek(f, 0, SEEK_END) != 0)
	{
		fclose(f);
		return 0;
	}

	int success = write_record(f, o) && fflush(f) == 0;

	if (fclose(f) != 0)
		success = 0;

	return success;
}

const char *status_text(int status)
{
	static const char *names[STATUS_COUNT] = {
			"Pending",
			"Preparing",
			"Ready",
			"Completed",
			"Cancelled"};

	if (status < 0 || status >= STATUS_COUNT)
		return "Unknown";

	return names[status];
}

static const char *payment_method_text(int method)
{
	switch (method)
	{
	case METHOD_CASH:
		return "Cash";
	case METHOD_CARD:
		return "Card";
	case METHOD_BANK_TRANSFER:
		return "Bank Transfer";
	case METHOD_OTHER:
		return "Other";
	default:
		return "None";
	}
}

static void print_money(int64_t centavos)
{
	printf("PHP %" PRId64 ".%02" PRId64,
				 centavos / 100, centavos % 100);
}

void print_order_summary(const Order *o)
{
	printf("\n========== ORDER #%" PRId64 " ==========\n",
				 o->orderNo);
	printf("Customer: %s\n", o->customer);
	printf("Date: %s\n", o->date);
	printf("Status: %s\n", status_text(o->status));
	printf("Payment: %s (%s)\n",
				 o->paymentStatus == PAYMENT_PAID ? "Paid" : "Unpaid",
				 payment_method_text(o->paymentMethod));

	puts("----------------------------------------");

	for (int i = 0; i < o->itemCount; i++)
	{
		const OrderItem *item = &o->items[i];

		printf("%-25s x%-3d ", item->name, item->quantity);
		print_money(item->subtotalCentavos);
		putchar('\n');
	}

	puts("----------------------------------------");
	printf("TOTAL: ");
	print_money(o->totalCentavos);
	puts("\n========================================");
}

void create_order(const Food foods[], int count)
{
	Order o = {0};
	char customer[50];

	if (count <= 0)
	{
		puts("No menu items are available.");
		return;
	}

	read_nonempty("Customer name: ", customer, sizeof customer);
	snprintf(o.customer, sizeof o.customer, "%s", customer);
	get_today(o.date);

	for (;;)
	{
		int availableCount = 0;

		puts("\n========== AVAILABLE ITEMS ==========");

		for (int i = 0; i < count; i++)
		{
			if (!foods[i].available)
				continue;

			printf("ID %d: %-25s PHP %" PRId64 ".%02" PRId64 "\n",
						 foods[i].id, foods[i].name,
						 foods[i].priceCentavos / 100,
						 foods[i].priceCentavos % 100);
			availableCount++;
		}

		if (!availableCount)
		{
			puts("There are no available items.");
			return;
		}

		if (o.itemCount >= MAX_ITEMS)
		{
			puts("Order item limit reached.");
			break;
		}

		int id = read_int("Food ID (0 to finish): ", 0, INT_MAX);

		if (id == 0)
			break;

		int index = find_food_index(foods, count, id);

		if (index < 0 || !foods[index].available)
		{
			puts("Invalid ID or item is sold out.");
			continue;
		}

		int quantity = read_int("Quantity: ", 1, 99);
		int existing = -1;

		for (int i = 0; i < o.itemCount; i++)
		{
			if (o.items[i].foodId == id)
			{
				existing = i;
				break;
			}
		}

		if (existing >= 0)
		{
			OrderItem *item = &o.items[existing];

			if (item->quantity > INT_MAX - quantity)
			{
				puts("Quantity is too large.");
				continue;
			}

			item->quantity += quantity;
			item->subtotalCentavos =
					item->unitPriceCentavos * item->quantity;
		}
		else
		{
			OrderItem *item = &o.items[o.itemCount++];

			item->foodId = foods[index].id;
			snprintf(item->name, sizeof item->name, "%s",
							 foods[index].name);
			item->unitPriceCentavos = foods[index].priceCentavos;
			item->quantity = quantity;
			item->subtotalCentavos =
					item->unitPriceCentavos * quantity;
		}

		o.totalCentavos = 0;

		for (int i = 0; i < o.itemCount; i++)
			o.totalCentavos += o.items[i].subtotalCentavos;

		printf("Current total: ");
		print_money(o.totalCentavos);
		putchar('\n');
	}

	if (o.itemCount == 0)
	{
		puts("Order cancelled; no items were added.");
		return;
	}

	print_order_summary(&o);

	if (!ask_yes_no("Save this order? (y/n): "))
	{
		puts("Order not saved.");
		return;
	}

	int existingOrders = count_orders();

	o.orderNo = (int64_t)existingOrders + 1;
	o.status = STATUS_PENDING;
	o.paymentStatus = PAYMENT_UNPAID;
	o.paymentMethod = METHOD_NONE;

	if (append_order(&o))
		printf("Order #%" PRId64 " saved successfully.\n", o.orderNo);
	else
		puts("Could not save the order. Check the storage file.");
}

static int allowed_transition(int current, int next)
{
	switch (current)
	{
	case STATUS_PENDING:
		return next == STATUS_PREPARING ||
					 next == STATUS_CANCELLED;

	case STATUS_PREPARING:
		return next == STATUS_PENDING ||
					 next == STATUS_READY ||
					 next == STATUS_CANCELLED;

	case STATUS_READY:
		return next == STATUS_PREPARING ||
					 next == STATUS_COMPLETED ||
					 next == STATUS_CANCELLED;

	case STATUS_COMPLETED:
		return next == STATUS_READY;

	case STATUS_CANCELLED:
	default:
		return 0;
	}
}

void view_orders(void)
{
	int count = count_orders();
	Order order;

	if (count == 0)
	{
		puts("No orders found.");
		return;
	}

	for (int i = 1; i <= count; i++)
	{
		if (read_order(i, &order))
		{
			print_order_summary(&order);
			puts("---------------");
		}
	}
}

void update_order_status(void)
{
	int totalOrders = count_orders();

	if (totalOrders == 0)
	{
		puts("No saved orders found.");
		return;
	}

	int64_t orderNo = read_int("Order number: ", 1, INT_MAX);
	Order o;

	if (!read_order(orderNo, &o))
	{
		puts("Order not found or storage could not be read.");
		return;
	}

	print_order_summary(&o);

	puts("\nAvailable statuses:");
	for (int i = 0; i < STATUS_COUNT; i++)
		printf("%d. %s\n", i + 1, status_text(i));

	int choice = read_int("New status: ", 1, STATUS_COUNT);
	int nextStatus = choice - 1;

	if (!allowed_transition(o.status, nextStatus))
	{
		printf("Transition from %s to %s is not allowed.\n",
					 status_text(o.status), status_text(nextStatus));
		return;
	}

	if (nextStatus == STATUS_CANCELLED &&
			!ask_yes_no("Cancel this order? (y/n): "))
		return;

	o.status = (OrderStatus)nextStatus;

	if (write_order(orderNo, &o))
		puts("Order status updated.");
	else
		puts("Could not save the status change.");
}
