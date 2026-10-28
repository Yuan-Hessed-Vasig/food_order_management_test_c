# Food Order Management System

A console-based Food Order Management System developed in C for managing cafeteria menu items, customer orders, order statuses, payments, and daily sales reports.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Main Menu](#main-menu)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Compilation and Execution](#compilation-and-execution)
- [Data Storage](#data-storage)
- [Order Status Workflow](#order-status-workflow)
- [Payment Management](#payment-management)
- [Daily Sales Report](#daily-sales-report)
- [Limitations](#limitations)

## Overview

The Food Order Management System is designed to help cafeteria staff manage basic food ordering and sales operations through a command-line interface.

The system allows staff to view and maintain the food menu, create orders containing multiple items, update order statuses, record payment information, and generate daily order and sales reports.

The program is written in **C** and uses file-based storage to retain menu and order records between program executions.

## Features

### 1. View Menu
- Display available menu items.
- View food names, categories, prices, and descriptions where available.
- Identify items that are sold out.

### 2. Manage Menu
- Add new food items.
- Edit existing food item information.
- Remove food items from the menu.
- Mark items as available or sold out.
- Save menu changes to a CSV file.

### 3. Create Order
- Record the customer's name.
- Select food items and specify quantities.
- Add multiple items to one order.
- Calculate item subtotals and the total order amount automatically.
- Save the order with its date, order number, and initial status.
- Store item names and prices in the order so historical records retain the details used when the order was created.

### 4. Update Order Status
- Find an existing order by its order number.
- Update its status according to the permitted status transitions.
- Prevent invalid status changes.
- Require confirmation when cancelling an order.

### 5. Record Payment
- Find an existing order.
- Record a payment method.
- Mark an order as paid or unpaid.
- Clear payment information when necessary.
- Prevent payment updates for cancelled orders.

Supported payment methods:
- Cash
- Card
- Bank Transfer
- Other
- Unpaid / no recorded payment

The program records payment information but does not process or verify actual transactions.

### 6. Daily Order/Sales Report
- Generate a report for a selected date.
- Display order and sales totals.
- Calculate collected payments and unpaid amounts.
- Summarize quantities of items sold.
- Count cancelled orders separately.
- Exclude cancelled orders from sales totals and item sales summaries.

## Main Menu

The program uses the following main menu options:

| Option | Function |
|---|---|
| 1 | View Menu |
| 2 | Manage Menu |
| 3 | Create Order |
| 4 | Update Order Status |
| 5 | Record Payment |
| 6 | Daily Order/Sales Report |
| 7 | Exit |

## Project Structure

The project is organized into separate C source files to keep related functions together.

| File | Purpose |
|---|---|
| `main.c` | Displays the main menu and directs the program to the selected function. |
| `food_system.h` | Defines data structures, constants, enumerations, and function declarations. |
| `utils.c` | Contains input handling, validation, and date utility functions. |
| `menu.c` | Handles menu loading, displaying, editing, and saving. |
| `order.c` | Handles order creation, storage, retrieval, summaries, and status updates. |
| `payment.c` | Handles payment information and payment status updates. |
| `report.c` | Generates daily order and sales reports. |
| `menu.csv` | Stores menu item information. |
| `orders_v2.dat` | Stores orders using the newer versioned binary storage format; created when orders are first saved. |
| `orders.dat` | Legacy order filename retained separately from the newer order file. |

## Requirements

- A C compiler that supports C11 or later, such as GCC.
- A terminal or command prompt.
- The project source files in the same directory.
- A `menu.csv` file, if the program is expected to load an existing menu.

## Compilation and Execution

### Compile using GCC

Open a terminal in the project directory and run:

```bash
gcc -std=c11 -Wall -Wextra main.c utils.c menu.c order.c payment.c report.c -o food_system
```

The command compiles the source files and produces an executable named `food_system`.

### Run on Windows

```bash
food_system.exe
```

### Run on Linux or macOS

```bash
./food_system
```

If compilation fails, check that all source files are present and that the function declarations in `food_system.h` match their implementations.

## Data Storage

The program uses local files instead of a database.

### Menu storage

Menu items are stored in `menu.csv`, which uses comma-separated values.

The menu data includes:
- Item ID
- Item name
- Category
- Price
- Availability

The program's data structure also supports a description field. The CSV loading and saving implementation must support that field for descriptions to persist between runs.

### Order storage

New orders are stored in `orders_v2.dat` using a versioned binary format.

Order records contain information such as:
- Order number
- Customer name
- Order date
- Ordered items and quantities
- Item prices and subtotals
- Total order amount
- Order status
- Payment status and method

**Important:** Keep `orders_v2.dat` and `menu.csv` when restarting the program if you want to retain the saved data. Back up these files before making changes to the project.

The legacy `orders.dat` file is separate from the new storage format. Existing legacy orders are not automatically migrated simply by changing the filename or compiling the updated program.

## Order Status Workflow

The system uses five order statuses:

- **Pending** — The order has been created and is awaiting preparation.
- **Preparing** — The order is being prepared.
- **Ready** — The order is ready for pickup or serving.
- **Completed** — The order has been fulfilled.
- **Cancelled** — The order has been cancelled.

Permitted status transitions:

| Current Status | Allowed Next Status |
|---|---|
| Pending | Preparing, Cancelled |
| Preparing | Pending, Ready, Cancelled |
| Ready | Preparing, Completed, Cancelled |
| Completed | Ready |
| Cancelled | None |

These transitions help prevent invalid status changes. A completed order can be returned to Ready to correct a mistaken completion, while a cancelled order cannot be reopened through the status workflow.

## Payment Management

Payment information is stored separately within each order record.

The system distinguishes between:
- **Payment status:** Paid or Unpaid.
- **Payment method:** Cash, Card, Bank Transfer, Other, or None.

An order can be paid before it reaches the Completed status, provided that it has not been cancelled.

Recording a payment only updates the program's records. It does not connect to a bank, payment gateway, or point-of-sale terminal.

## Daily Sales Report

The daily report uses the order date to identify records for the selected day.

The report summarizes:
- Total sales from non-cancelled orders.
- Payments recorded as collected.
- Amounts that remain unpaid.
- Quantities of items sold.
- Cancelled order counts.

Cancelled orders are reported separately and excluded from sales and item quantity totals.

Report accuracy depends on the saved order records being readable and consistent.

## Limitations

The system is intended for basic cafeteria operations and has the following limitations:

- It does not support online ordering.
- It does not process online or electronic payments.
- It cannot independently verify whether a payment was received.
- Inventory management is limited to item availability; it does not track stock quantities or automatically deduct ingredients.
- It does not support multiple users working simultaneously.
- It does not automatically manage food preparation or pickup.
- Data is stored in local files rather than a database server.
- The system does not provide a graphical user interface.
- Legacy order migration requires a separate migration procedure if older order records need to be preserved in the new format.

## Conclusion

The Food Order Management System demonstrates the use of procedural programming in C to solve a practical cafeteria management problem. It combines modular source files, structured data, input validation, file handling, order processing, payment recording, and basic reporting into one console-based application.
