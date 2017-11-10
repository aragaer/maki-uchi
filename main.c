#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "maki-uchi.h"

#define ONE_DAY (60*60*24)

static char stamp_buf[11];

char *format_stamp(time_t value) {
  value -= value % ONE_DAY;
  strftime(stamp_buf, sizeof(stamp_buf), "%Y.%m.%d", localtime(&value));
  return stamp_buf;
}

char *file_name = "test.data";
int parseable;

enum string_idx {
  DATE_SEP,
  SKIPPED,
  PERIOD_SEP,
  LAST_PERIOD_SEP,
  NOT_DONE,
  LAST,
  EARLIEST,
  DONE,
  NEVER,
  INCOMPLETE,
  INCOMPLETE2,
  INCOMPLETE3,
  MISSING1,
  MISSING2,
};

const char *human_readable[] = {
  [DATE_SEP] = " to ",
  [SKIPPED] = "You skipped",
  [PERIOD_SEP] = ", ",
  [LAST_PERIOD_SEP] = " and ",
  [NOT_DONE] = "You did not do your maki-uchi today",
  [LAST] = "The last date you did your maki-uchi is",
  [EARLIEST] = "The earliest date you did your maki-uchi is",
  [DONE] = "You did your maki-uchi today",
  [NEVER] = "You did not do maki-uchi at all",
  [INCOMPLETE] = "You only did",
  [INCOMPLETE2] = "maki-uchi today",
  [INCOMPLETE3] = "maki-uchi on",
  [MISSING1] = "You should do",
  [MISSING2] = "more maki-uchi",
};

const char *computer_readable[] = {
  [DATE_SEP] = "-",
  [SKIPPED] = "skipped",
  [PERIOD_SEP] = " ",
  [LAST_PERIOD_SEP] = " ",
  [LAST] = "last",
  [EARLIEST] = "earliest",
  [DONE] = "last today",
  [NEVER] = "last never",
  [INCOMPLETE] = "only",
  [INCOMPLETE3] = "on",
};

const char * const *_ = human_readable;

void print_period(time_t start, time_t end) {
  printf("%s", format_stamp(start+1));
  if (end > start + 1 + ONE_DAY)
    printf("%s%s", _[DATE_SEP], format_stamp(end-1));
}

void print_skipped(maki_uchi_log_t *log) {
  log_entry_t *entry = log_get_entry_before(log, NULL);
  log_entry_t *next = log_get_entry_before(log, entry);
  log_entry_t *earliest = log_get_first_entry(log);
  printf("%s ", _[SKIPPED]);
  while (next) {
    print_period(next->end, entry->start);
    entry = next;
    next = log_get_entry_before(log, entry);
    if (next == earliest)
      printf("%s", _[LAST_PERIOD_SEP]);
    else if (next != NULL)
      printf("%s", _[PERIOD_SEP]);
  }
  printf("\n");
}

void print_incomplete(maki_uchi_log_t *log) {
  int count;
  for (count = 1; count < DAILY_REQUIREMENT; count++) {
    log_entry_t *entry = log_get_entry_before(log, NULL);
    int header_printed = 0;
    for (; entry; entry = log_get_entry_before(log, entry))
      if (entry->count == count) {
	time_t displayed_end = entry->end;
	if (!parseable && time(NULL) < displayed_end) // already printed as today count
	  displayed_end -= ONE_DAY;
	if (entry->start > displayed_end)
	  continue;
	if (header_printed)
	  printf("%s", _[PERIOD_SEP]);
	else
	  printf("%s %d %s ", _[INCOMPLETE], count, _[INCOMPLETE3]);
	header_printed = 1;
	print_period(entry->start, displayed_end);
      }
    if (header_printed)
      printf("\n");
  }
}

void usage() {
  fprintf(stderr, "Usage: maki-uchi [-f filename] [number]\n");
  exit(EXIT_FAILURE);
}

void store_log(maki_uchi_log_t *log) {
  int fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_write_file(log, fd);
  close(fd);
}

void print_log(maki_uchi_log_t *log) {
  int today_count = log_status(log, time(NULL));
  if (today_count == 0) {
    if (!parseable)
      printf("%s\n", _[NOT_DONE]);
    log_entry_t *entry = log_get_last_entry(log);
    if (entry == NULL)
      printf("%s\n", _[NEVER]);
    else
      printf("%s %s\n", _[LAST], format_stamp(entry->end));
  } else if (today_count == DAILY_REQUIREMENT)
    printf("%s\n", _[DONE]);
  else if (!parseable)
    printf("%s %d %s\n%s %d %s\n", _[INCOMPLETE], today_count, _[INCOMPLETE2],
	   _[MISSING1], DAILY_REQUIREMENT - today_count, _[MISSING2]);
  log_entry_t *first = log_get_first_entry(log);
  if (first != log_get_last_entry(log))
    print_skipped(log);
  print_incomplete(log);
  if (first != NULL)
    printf("%s %s\n", _[EARLIEST], format_stamp(first->start));
}

void parse_args(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "pf:")) != -1)
    switch (opt) {
    case 'f':
      file_name = optarg;
      break;
    case 'p':
      parseable = 1;
      _ = computer_readable;
      break;
    default:
      usage();
      break;
    }
}

int main(int argc, char *argv[]) {
  maki_uchi_log_t log;
  log_init(&log);
  parse_args(argc, argv);
  int fd = open(file_name, O_RDONLY);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_read_file(&log, fd);
  close(fd);
  if (optind < argc) {
    int count;
    if (sscanf(argv[optind], "%d", &count) != 1)
      usage();
    log_add(&log, count, time(NULL));
    store_log(&log);
  } else
    print_log(&log);
}
