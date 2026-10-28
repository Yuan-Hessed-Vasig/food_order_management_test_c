# Food Order Management System

A console-based Food Order Management System developed in C. The program allows users to manage a food menu, create customer orders, update order statuses, record payments, and generate daily order and sales reports.

## Features

1. **View Menu** — Display food names, descriptions, categories, prices, and availability.
2. **Manage Menu** — Add, edit, remove, and update the availability of food items.
3. **Create Order** — Create orders containing multiple food items and quantities, calculate subtotals and the total amount, and save confirmed orders.
4. **Update Order Status** — Update an order's status according to permitted status transitions.
5. **View Orders** — Display saved orders, including customer information, ordered items, totals, statuses, and payment details.
6. **Record Payment** — Record payment methods and update payment status.
7. **Daily Order/Sales Report** — Generate a report based on saved order records.
8. **Exit** — Close the program.

## Technologies Used

- **Language:** C
- **Compiler:** GCC
- **Standard:** C11
- **Menu storage:** CSV
- **Order storage:** Binary data file

## Project Structure

```text
Food-Order-Management-System/
├── main.c              # Main menu and program entry point
├── food_system.h       # Shared structures, constants, and declarations
├── utils.c             # Input validation and utility functions
├── menu.c              # Menu management and CSV operations
├── order.c             # Order creation, storage, viewing, and status updates
├── payment.c           # Payment recording
├── report.c            # Daily order and sales report
├── menu.csv            # Persistent menu data
├── orders_v2.dat       # Persistent order records
└── README.md
```

## Requirements

- GCC compiler
- Terminal or command-line environment
- Windows with Git Bash, Linux, or another environment that supports GCC

## Compilation

Open a terminal in the project directory and run:

```bash
gcc -std=c11 -Wall -Wextra main.c utils.c menu.c order.c payment.c report.c -o food_system
```

## Running the Program

In Git Bash or a compatible terminal:

```bash
./food_system
```

On Windows, the compiled executable is `food_system.exe` and can also be run directly from the terminal.

## Data Storage

### `menu.csv`

Stores menu information, including food IDs, names, descriptions, categories, prices, and availability. Menu changes are saved to this file so they can persist between program runs.

### `orders_v2.dat`

Stores order records, including order numbers, customer information, dates, ordered items, quantities, totals, order statuses, and payment information.

The `.dat` file uses a binary format and is not intended to be edited manually.

**Resetting order data:** Close the program and back up or delete `orders_v2.dat` if you want to start a fresh demonstration. This removes the saved order history. The program can initialize a new order data file when a new order is saved.

## Order Statuses

The system supports the following statuses:

- Pending
- Preparing
- Ready
- Completed
- Cancelled

Status changes are restricted to permitted transitions to prevent invalid updates.

## Payment Statuses

- **Unpaid** — Payment has not been recorded as paid.
- **Paid** — Payment has been recorded in the system.

Available payment methods include cash, card, bank transfer, and other.

## Limitations

- The application runs in a command-line interface.
- It does not support online ordering or online payment processing.
- Recording a payment does not independently verify that a transaction occurred.
- Inventory quantities are not automatically tracked.
- The system does not provide multi-user access or concurrent order processing.
- Order fulfillment and preparation are managed manually.
- Order records use a binary file format rather than a database.

## Purpose

This project demonstrates fundamental C programming concepts, including functions, structures, enumerations, file handling, input validation, conditional statements, loops, and modular program organization.
