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

enum STRINGS {
  DATE_SEP,
  SKIPPED,
  PERIOD_SEP,
  LAST_PERIOD_SEP,
  NOT_DONE,
  LAST,
  EARLIEST,
};

const char *human_readable[] = {
  [DATE_SEP] = " to ",
  [SKIPPED] = "You skipped",
  [PERIOD_SEP] = ", ",
  [LAST_PERIOD_SEP] = " and ",
  [NOT_DONE] = "You did not do your maki-uchi today",
  [LAST] = "The last date you did your maki-uchi is",
  [EARLIEST] = "The earliest date you did your maki-uchi is",
};

const char *computer_readable[] = {
  [DATE_SEP] = "-",
  [SKIPPED] = "skipped",
  [PERIOD_SEP] = " ",
  [LAST_PERIOD_SEP] = " ",
  [NOT_DONE] = "not done",
  [LAST] = "last",
  [EARLIEST] = "earliest",
};

const char **_ = human_readable;

void print_skipped_between(log_entry_t *before, log_entry_t *after) {
  printf("%s", format_stamp(before->end+1));
  if (after->start != before->end + ONE_DAY + 1)
    printf("%s%s", _[DATE_SEP], format_stamp(after->start-1));
}

void print_skipped(maki_uchi_log_t *log) {
  log_entry_t *entry = log_get_entry_before(log, NULL);
  log_entry_t *next = log_get_entry_before(log, entry);
  log_entry_t *earliest = log_get_first_entry(log);
  printf("%s ", _[SKIPPED]);
  while (next) {
    print_skipped_between(next, entry);
    entry = next;
    next = log_get_entry_before(log, entry);
    if (next == earliest)
      printf("%s", _[LAST_PERIOD_SEP]);
    else if (next != NULL)
      printf("%s", _[PERIOD_SEP]);
  }
  printf("\n");
}

void usage() {
  fprintf(stderr, "Usage: maki-uchi [-f filename] [number]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  maki_uchi_log_t log;
  log_init(&log);
  time_t now = time(NULL);
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
  int fd = open(file_name, O_RDONLY);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_read_file(&log, fd);
  close(fd);
  if (optind < argc) {
    int fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1 && errno != ENOENT)
      perror("open");
    int count;
    if (sscanf(argv[optind], "%d", &count) != 1)
      usage();
    log_add(&log, count, now);
    log_write_file(&log, fd);
    close(fd);
  } else {
    if (log_status(&log, now) == 0) {
      printf("%s\n", _[NOT_DONE]);
      log_entry_t *entry = log_get_last_entry(&log);
      if (entry == NULL)
	printf("You did not do maki-uchi at all\n");
      else
	printf("%s %s\n", _[LAST], format_stamp(entry->end));
    } else
      printf("You did your maki-uchi today\n");
    log_entry_t *first = log_get_first_entry(&log);
    if (first != log_get_last_entry(&log))
      print_skipped(&log);
    if (first != NULL)
      printf("%s %s\n", _[EARLIEST], format_stamp(first->start));
  }
}
