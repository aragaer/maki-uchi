#include <stdio.h>

#include "computer.h"
#include "maki-uchi.h"
#include "print.h"

static void print_period(time_t start, time_t end) {
  printf("%s", format_stamp(start+1));
  if (end > start + 1 + ONE_DAY)
    printf("-%s", format_stamp(end-1));
}

static void print_skipped(maki_uchi_log_t *log) {
  log_entry_t *entry = log_get_entry_before(log, NULL);
  log_entry_t *next = log_get_entry_before(log, entry);
  printf("skipped ");
  while (next) {
    print_period(next->end, entry->start);
    entry = next;
    next = log_get_entry_before(log, entry);
    if (next != NULL)
      printf(" ");
  }
  printf("\n");
}

static void print_incomplete(maki_uchi_log_t *log) {
  int count;
  for (count = 1; count < DAILY_REQUIREMENT; count++) {
    log_entry_t *entry = log_get_entry_before(log, NULL);
    int header_printed = 0;
    for (; entry; entry = log_get_entry_before(log, entry))
      if (entry->count == count) {
	time_t displayed_end = entry->end;
	if (entry->start > displayed_end)
	  continue;
	if (header_printed)
	  printf(" ");
	else
	  printf("only %d on ", count);
	header_printed = 1;
	print_period(entry->start, displayed_end);
      }
    if (header_printed)
      printf("\n");
  }
}

static void print_last(maki_uchi_log_t *log) {
  int today_count = log_status(log, time(NULL));
  if (today_count == 0) {
    log_entry_t *entry = log_get_last_entry(log);
    if (entry == NULL)
      printf("last never\n");
    else
      printf("last %s\n", format_stamp(entry->end));
  } else if (today_count == DAILY_REQUIREMENT)
    printf("last today\n");
}

void print_log_computer(maki_uchi_log_t *log) {
  log_entry_t *first = log_get_first_entry(log);
  print_last(log);
  if (first != log_get_last_entry(log))
    print_skipped(log);
  print_incomplete(log);
  if (first != NULL)
    printf("earliest %s\n", format_stamp(first->start));
}

