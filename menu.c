
#include "food_system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define DESCRIPTION_SIZE 100

static void clean_field(char *s)
{
	while (*s)
	{
		if (*s == ',' || *s == '\n' || *s == '\r')
			*s = ' ';
		s++;
	}
}

static int parse_price(const char *s, int64_t *price)
{
	char *end;
	double value = strtod(s, &end);

	if (s == end || *end != '\0' || value < 0 || value > 90000000000000.0)
		return 0;

	/* Accept at most two decimal places. */
	const char *dot = strchr(s, '.');
	if (dot && strlen(dot + 1) > 2)
		return 0;

	*price = (int64_t)(value * 100.0 + 0.5);
	return 1;
}

static void print_price(int64_t centavos)
{
	printf("₱%" PRId64 ".%02" PRId64,
				 centavos / 100, centavos % 100);
}

static int read_price(int64_t *price)
{
	char input[64];

	for (;;)
	{
		read_line("Price (e.g. 99.50): ", input, sizeof input);

		if (parse_price(input, price))
			return 1;

		puts("Invalid price. Use a non-negative amount with up to 2 decimals.");
	}
}

static void default_menu(Food foods[], int *count)
{
	static const Food defaults[] = {
			{1, "Fried Chicken/Rice", "Chicken with rice", "Rice Meals", 9900, true},
			{2, "Plain Rice", "Steamed white rice", "Extra", 2500, true},
			{3, "Fried Rice/3pcs Siomai", "Fried rice with three siomai", "Rice Meals", 8000, true},
			{4, "Shawarma Rice", "Rice topped with shawarma", "Rice Meals", 9800, true},
			{5, "Burger Siomaitake/Rice", "Burger siomaitake with rice", "Rice Meals", 8900, true},
			{6, "Burger", "Classic burger", "Snacks", 5000, true},
			{7, "Fries", "Fried potato sticks", "Snacks", 4500, true},
			{8, "Pizza", "Pizza slice", "Snacks", 9900, true},
			{9, "Ice Cream", "Ice cream serving", "Desserts", 4000, true},
			{10, "Soft Drinks", "Cold soft drink", "Drinks", 3000, true}};

	*count = (int)(sizeof defaults / sizeof defaults[0]);
	memcpy(foods, defaults, sizeof defaults);
}

int load_menu(Food foods[])
{
	FILE *file = fopen(MENU_FILE, "r");
	char line[512];
	int count = 0;

	if (!file)
	{
		default_menu(foods, &count);
		save_menu(foods, count);
		return count;
	}

	/* Skip the CSV header. */
	if (!fgets(line, sizeof line, file))
	{
		fclose(file);
		default_menu(foods, &count);
		save_menu(foods, count);
		return count;
	}

	while (fgets(line, sizeof line, file) && count < MAX_FOODS)
	{
		Food f = {0};
		char *fields[6];
		char *p = line;
		int n = 0;
		int old_format;
		int available;

		while (n < 6)
		{
			fields[n++] = p;
			char *comma = strchr(p, ',');
			if (!comma)
				break;
			*comma = '\0';
			p = comma + 1;
		}

		old_format = (n == 5);

		if ((!old_format && n != 6) ||
				!parse_int(fields[0], &f.id) || f.id <= 0)
			continue;

		if (old_format)
		{
			snprintf(f.name, sizeof f.name, "%s", fields[1]);
			snprintf(f.description, sizeof f.description, "%s", fields[1]);
			snprintf(f.category, sizeof f.category, "%s", fields[2]);

			if (!parse_price(fields[3], &f.priceCentavos) ||
					!parse_int(fields[4], &available))
				continue;
		}
		else
		{
			snprintf(f.name, sizeof f.name, "%s", fields[1]);
			snprintf(f.description, sizeof f.description, "%s", fields[2]);
			snprintf(f.category, sizeof f.category, "%s", fields[3]);

			if (!parse_price(fields[4], &f.priceCentavos) ||
					!parse_int(fields[5], &available))
				continue;
		}

		if (!f.name[0] || !f.category[0] ||
				(available != 0 && available != 1))
			continue;

		f.available = available != 0;
		foods[count++] = f;
	}

	fclose(file);
	return count;
}

void save_menu(const Food foods[], int count)
{
	FILE *file = fopen(MENU_FILE, "w");

	if (!file)
	{
		perror("Could not save menu");
		return;
	}

	fprintf(file, "id,name,description,category,price,available\n");

	for (int i = 0; i < count; i++)
	{
		Food f = foods[i];

		clean_field(f.name);
		clean_field(f.description);
		clean_field(f.category);

		fprintf(file, "%d,%s,%s,%s,%" PRId64 ".%02" PRId64 ",%d\n",
						f.id, f.name, f.description, f.category,
						f.priceCentavos / 100, f.priceCentavos % 100,
						f.available ? 1 : 0);
	}

	fclose(file);
}

int find_food_index(const Food foods[], int count, int id)
{
	for (int i = 0; i < count; i++)
		if (foods[i].id == id)
			return i;

	return -1;
}

void display_menu_items(const Food foods[], int count)
{
	for (int i = 0; i < count; i++)
	{
		printf("%3d. %-28s ", foods[i].id, foods[i].name);
		print_price(foods[i].priceCentavos);
		printf("  [%s]\n", foods[i].available ? "Available" : "Sold out");
		printf("     Category: %s\n", foods[i].category);
		printf("     %s\n", foods[i].description);
	}
}

void view_menu(const Food foods[], int count)
{
	puts("\n========== FOOD MENU ==========");

	if (count == 0)
		puts("No menu items.");
	else
		display_menu_items(foods, count);

	press_any_key();
}

static int next_food_id(const Food foods[], int count)
{
	int maxId = 0;

	for (int i = 0; i < count; i++)
		if (foods[i].id > maxId)
			maxId = foods[i].id;

	return maxId == 2147483647 ? 0 : maxId + 1;
}

static void add_food(Food foods[], int *count)
{
	Food f = {0};

	if (*count >= MAX_FOODS)
	{
		puts("Menu is full.");
		return;
	}

	f.id = next_food_id(foods, *count);
	if (!f.id)
	{
		puts("No more food IDs available.");
		return;
	}

	read_nonempty("Food name: ", f.name, sizeof f.name);
	read_nonempty("Description: ", f.description, sizeof f.description);
	read_nonempty("Category: ", f.category, sizeof f.category);
	read_price(&f.priceCentavos);
	f.available = true;

	foods[(*count)++] = f;
	save_menu(foods, *count);
	puts("Food added.");
}

static void edit_food(Food foods[], int count)
{
	int id = read_int("Food ID to edit: ", 1, 2147483647);
	int i = find_food_index(foods, count, id);

	if (i < 0)
	{
		puts("Food not found.");
		return;
	}

	read_nonempty("New name: ", foods[i].name, sizeof foods[i].name);
	read_nonempty("New description: ", foods[i].description,
								sizeof foods[i].description);
	read_nonempty("New category: ", foods[i].category,
								sizeof foods[i].category);
	read_price(&foods[i].priceCentavos);

	save_menu(foods, count);
	puts("Food updated.");
}

static void remove_food(Food foods[], int *count)
{
	int id = read_int("Food ID to remove: ", 1, 2147483647);
	int i = find_food_index(foods, *count, id);

	if (i < 0)
	{
		puts("Food not found.");
		return;
	}

	if (!ask_yes_no("Remove this item? (y/n): "))
		return;

	for (int j = i; j < *count - 1; j++)
		foods[j] = foods[j + 1];

	(*count)--;
	save_menu(foods, *count);
	puts("Food removed. Existing orders are unaffected.");
}

static void toggle_availability(Food foods[], int count)
{
	int id = read_int("Food ID: ", 1, 2147483647);
	int i = find_food_index(foods, count, id);

	if (i < 0)
	{
		puts("Food not found.");
		return;
	}

	foods[i].available = !foods[i].available;
	save_menu(foods, count);

	printf("%s is now %s.\n", foods[i].name,
				 foods[i].available ? "available" : "sold out");
}

void manage_menu(Food foods[], int *count)
{
	int choice;

	do
	{
		puts("\n========== MANAGE MENU ==========");
		puts("1. View menu");
		puts("2. Add food");
		puts("3. Edit food");
		puts("4. Remove food");
		puts("5. Toggle availability");
		puts("6. Return");

		choice = read_int("Choice: ", 1, 6);

		switch (choice)
		{
		case 1:
			display_menu_items(foods, *count);
			break;
		case 2:
			add_food(foods, count);
			break;
		case 3:
			edit_food(foods, *count);
			break;
		case 4:
			remove_food(foods, count);
			break;
		case 5:
			toggle_availability(foods, *count);
			break;
		case 6:
			break;
		}
	} while (choice != 6);
}
