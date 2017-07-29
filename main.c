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

int main(int argc, char *argv[] __attribute__((unused))) {
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
    log_add(&log, 10, now);
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
    log_entry_t *entry = log_get_first_entry(&log);
    if (entry != NULL)
      printf("The earliest date you did your maki-uchi is %s\n",
	     format_stamp(entry->start));
  }
}
