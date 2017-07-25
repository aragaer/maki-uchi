#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "maki-uchi.h"


int main(int argc, char *argv[] __attribute__((unused))) {
  maki_uchi_log_t log;
  log_init(&log);
  time_t now = time(NULL);
  if (argc == 2) {
    int fd = open("test.data", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1 && errno != ENOENT)
      perror("open");
    log_add(&log, 10, now);
    log_write_file(&log, fd);
    close(fd);
  } else {
    int fd = open("test.data", O_RDONLY);
    if (fd == -1 && errno != ENOENT)
      perror("open");
    log_read_file(&log, fd);
    if (log_status(&log, now) == 0)
      printf("You did not do your maki-uchi today\n");
    else
      printf("You did your maki-uchi today\n");
    close(fd);
  }
}
