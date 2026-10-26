#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include "food_system.h"

/* Read one line from the keyboard (safe replacement for scanf).
	 Strips the newline and discards anything that did not fit in buf. */
void read_line(const char *prompt, char *buf, int size)
{
	if (prompt != NULL)
		printf("%s", prompt);

	if (fgets(buf, size, stdin) == NULL)
	{
		printf("\nInput closed. Exiting.\n");
		exit(0);
	}

	if (strchr(buf, '\n') == NULL)
	{
		int c;
		while ((c = getchar()) != '\n' && c != EOF)
			;
	}
	buf[strcspn(buf, "\r\n")] = '\0';
}

int parse_int(const char *s, int *out)
{
	char *end;
	long v;

	if (s == NULL || *s == '\0')
		return 0;

	v = strtol(s, &end, 10);
	if (end == s)
		return 0;
	while (isspace((unsigned char)*end))
		end++;
	if (*end != '\0')
		return 0;
	if (v < INT_MIN || v > INT_MAX)
		return 0;

	*out = (int)v;
	return 1;
}

int parse_double(const char *s, double *out)
{
	char *end;
	double v;

	if (s == NULL || *s == '\0')
		return 0;

	v = strtod(s, &end);
	if (end == s)
		return 0;
	while (isspace((unsigned char)*end))
		end++;
	if (*end != '\0')
		return 0;

	*out = v;
	return 1;
}

/* Keeps asking until the user types a whole number from min to max. */
int read_int(const char *prompt, int min, int max)
{
	char buf[64];
	int v;

	for (;;)
	{
		read_line(prompt, buf, sizeof buf);
		if (parse_int(buf, &v) && v >= min && v <= max)
			return v;
		printf("Invalid input. Enter a number from %d to %d.\n", min, max);
	}
}

double read_double(const char *prompt, double min)
{
	char buf[64];
	double v;

	for (;;)
	{
		read_line(prompt, buf, sizeof buf);
		if (parse_double(buf, &v) && v >= min)
			return v;
		printf("Invalid input. Enter a number that is at least %.2f.\n", min);
	}
}

void read_nonempty(const char *prompt, char *buf, int size)
{
	for (;;)
	{
		read_line(prompt, buf, size);
		if (buf[0] != '\0')
			return;
		printf("This cannot be empty.\n");
	}
}

int ask_yes_no(const char *prompt)
{
	char buf[16];

	for (;;)
	{
		read_line(prompt, buf, sizeof buf);
		if (buf[0] == 'Y' || buf[0] == 'y')
			return 1;
		if (buf[0] == 'N' || buf[0] == 'n')
			return 0;
		printf("Please type Y or N.\n");
	}
}

void press_any_key(void)
{
	char buf[16];
	printf("\nPress Enter to return to main menu...");
	read_line(NULL, buf, sizeof buf);
}

void get_today(char out[11])
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(out, 11, "%Y-%m-%d", t);
}
