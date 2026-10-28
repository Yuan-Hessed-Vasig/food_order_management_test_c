
#ifndef FOOD_SYSTEM_H
#define FOOD_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_FOODS 100
#define MAX_ITEMS 20

#define MENU_FILE "menu.csv"
#define ORDER_FILE "orders_v2.dat"
#define LEGACY_ORDER_FILE "orders.dat"

typedef enum
{
	STATUS_PENDING,
	STATUS_PREPARING,
	STATUS_READY,
	STATUS_COMPLETED,
	STATUS_CANCELLED,
	STATUS_COUNT
} OrderStatus;

typedef enum
{
	PAYMENT_UNPAID,
	PAYMENT_PAID
} PaymentStatus;

typedef enum
{
	METHOD_NONE,
	METHOD_CASH,
	METHOD_CARD,
	METHOD_BANK_TRANSFER,
	METHOD_OTHER
} PaymentMethod;

typedef struct
{
	int id;
	char name[50];
	char description[100];
	char category[50];
	int64_t priceCentavos;
	bool available;
} Food;

typedef struct
{
	int foodId;
	char name[50];
	int64_t unitPriceCentavos;
	int quantity;
	int64_t subtotalCentavos;
} OrderItem;

typedef struct
{
	int64_t orderNo;
	char customer[50];
	char date[11];

	OrderItem items[MAX_ITEMS];
	int itemCount;

	int64_t totalCentavos;
	OrderStatus status;
	PaymentStatus paymentStatus;
	PaymentMethod paymentMethod;
} Order;

/* Input utilities */
void read_line(const char *prompt, char *buf, int size);
int parse_int(const char *s, int *out);
int parse_double(const char *s, double *out);
int read_int(const char *prompt, int min, int max);
double read_double(const char *prompt, double min);
void read_nonempty(const char *prompt, char *buf, int size);
int ask_yes_no(const char *prompt);
void press_any_key(void);
void get_today(char out[11]);

/* Menu management */
int load_menu(Food foods[]);
void save_menu(const Food foods[], int count);
int find_food_index(const Food foods[], int count, int id);
void display_menu_items(const Food foods[], int count);
void view_menu(const Food foods[], int count);
void manage_menu(Food foods[], int *count);

/* Order management */
const char *status_text(int status);
int count_orders(void);
int read_order(int64_t orderNo, Order *o);
int write_order(int64_t orderNo, const Order *o);
int append_order(const Order *o);
void print_order_summary(const Order *o);
void create_order(const Food foods[], int count);
void update_order_status(void);
void view_orders(void);

/* Payment */
void record_payment(void);

/* Reports */
void daily_sales_order_report(void);

#endif
