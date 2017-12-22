#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "maki-uchi.h"
#include "minunit.h"


static time_t timestamp = 0;

char *test_read_file() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_read(log, "1970.01.01", 10);

  char filename[] = "/tmp/test-maki-uchiXXXXXX";
  int fd = mkstemp(filename);
  int result = log_read_file(log, fd);
  mu_assert("Read 1 success", result == 0);
  mu_assert("Not done", log_status(log, timestamp) == 0);

  write(fd, "1970.01.01", 10);
  lseek(fd, 0, SEEK_SET);
  result = log_read_file(log, fd);
  mu_assert("Read 2 success", result == 10);
  mu_assert("Done", log_status(log, timestamp) == 10);

  lseek(fd, 0, SEEK_SET);
  write(fd, "1970.01.0X", 10);
  result = log_read_file(log, fd);
  mu_assert("Read 3 fail", result == -1);

  close(fd);
  unlink(filename);

  result = log_read_file(log, fd);
  mu_assert("Read 4 fail", result == -1);

  log_release(log);
  return NULL;
}

char *test_write_file() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);

  char filename[] = "/tmp/test-maki-uchiXXXXXX";
  int fd = mkstemp(filename);
  write(fd, "1970.01.10", 10);
  lseek(fd, 0, SEEK_SET);
  int result = log_write_file(log, fd);
  mu_assert("Write 1 success", result == 0);
  mu_assert("File is empty", lseek(fd, 0, SEEK_END) == 0);

  log_read(log, "1970.01.01", 10);
  lseek(fd, 0, SEEK_SET);
  result = log_write_file(log, fd);
  mu_assert("Write 2 success", result == 14);
  mu_assert("File has correct size", lseek(fd, 0, SEEK_END) == result);
  void *data = mmap(NULL, result, PROT_READ, MAP_PRIVATE, fd, 0);
  mu_assert("mmap success", data != NULL);
  mu_assert("File has correct contents", memcmp(data, "1970.01.01 10\n", result) == 0);

  log_read(log, "1970.01.01-1970.01.02", 21);
  lseek(fd, 0, SEEK_SET);
  result = log_write_file(log, fd);
  mu_assert("Write 3 success", result == 25);
  mu_assert("File has correct size", lseek(fd, 0, SEEK_END) == result);
  data = mmap(NULL, result, PROT_READ, MAP_PRIVATE, fd, 0);
  mu_assert("mmap success", data != NULL);
  mu_assert("File has correct contents",
	    memcmp(data, "1970.01.01-1970.01.02 10\n", result) == 0);

  close(fd);
  unlink(filename);

  result = log_write_file(log, fd);
  mu_assert("Write 4 fail", result == -1);

  log_release(log);
  return NULL;
}
