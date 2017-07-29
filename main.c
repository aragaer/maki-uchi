#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
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

void print_skipped_between(log_entry_t *before, log_entry_t *after) {
  printf("%s", format_stamp(before->end+1));
  if (after->start != before->end + ONE_DAY + 1)
    printf(" to %s", format_stamp(after->start-1));
}

void print_skipped(maki_uchi_log_t *log) {
  log_entry_t *entry = log_get_entry_before(log, NULL);
  log_entry_t *next = log_get_entry_before(log, entry);
  log_entry_t *earliest = log_get_first_entry(log);
  printf("You skipped ");
  while (next) {
    print_skipped_between(next, entry);
    entry = next;
    next = log_get_entry_before(log, entry);
    if (next == earliest)
      printf(" and ");
    else if (next != NULL)
      printf(", ");
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  maki_uchi_log_t log;
  log_init(&log);
  time_t now = time(NULL);
  int fd = open("test.data", O_RDONLY);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_read_file(&log, fd);
  close(fd);
  if (argc == 2) {
    int fd = open("test.data", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1 && errno != ENOENT)
      perror("open");
    int count;
    if (sscanf(argv[1], "%d", &count) != 1) {
      printf("Usage: maki-uchi [number]\n");
      exit(-1);
    }
    log_add(&log, count, now);
    log_write_file(&log, fd);
    close(fd);
  } else {
    if (log_status(&log, now) == 0) {
      printf("You did not do your maki-uchi today\n");
      log_entry_t *entry = log_get_last_entry(&log);
      if (entry == NULL) {
	printf("You did not do maki-uchi at all\n");
      } else
	printf("The last date you did your maki-uchi is %s\n",
	       format_stamp(entry->end));
    } else
      printf("You did your maki-uchi today\n");
    log_entry_t *first = log_get_first_entry(&log);
    if (first != log_get_last_entry(&log))
      print_skipped(&log);
    if (first != NULL)
      printf("The earliest date you did your maki-uchi is %s\n",
	     format_stamp(first->start));
  }
}
