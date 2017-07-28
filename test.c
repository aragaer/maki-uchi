#define _XOPEN_SOURCE 500
#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "maki-uchi.h"
#include "minunit.h"

#ifdef TEST_DEBUG
#define debug(...) printf(__VA_ARGS__)
#define dump(...) dump_log(__VA_ARGS__)
#else
#define debug(...)
#define dump(...)
#endif
 
int tests_run;

#define ONE_DAY (24 * 60 * 60)

time_t timestamp = 0;
char buf[1024], expected[1024];

static char *test_log() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  mu_assert("Nothing for day 0", log_status(log, timestamp) == 0);
  debug("Doing 10 maki-uchi on day 0\n");
  log_add(log, 10, timestamp);
  mu_assert("Done for day 0", log_status(log, timestamp) == 10);
  mu_assert("Not done for day -1", log_status(log, timestamp - ONE_DAY) == 0);
  mu_assert("Not done for day 1", log_status(log, timestamp + ONE_DAY) == 0);
  log_add(log, 10, timestamp + ONE_DAY);
  debug("Doing 10 maki-uchi on day 1\n");
  mu_assert("Done for day 0", log_status(log, timestamp) == 10);
  mu_assert("Not done for day -1", log_status(log, timestamp - ONE_DAY) == 0);
  mu_assert("Done for day 1", log_status(log, timestamp + ONE_DAY) == 10);
  log_release(log);
  return NULL;
}

static char *test_one_day() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_add(log, 10, timestamp + ONE_DAY*3/4);
  dump(log);
  mu_assert("before midnight", log_status(log, timestamp - 300) == 0);
  mu_assert("after midnight", log_status(log, timestamp + 300) == 10);
  mu_assert("in 5 minutes", log_status(log, timestamp + ONE_DAY/2 + 300) == 10);
  mu_assert("5 minutes earlier", log_status(log, timestamp + ONE_DAY/2 + 300) == 10);
  log_release(log);
  return NULL;
}

static char *test_log_multiple_days() {
  int i;
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  for (i = 0; i < 10; i++)
    log_add(log, 10, timestamp + ONE_DAY * i * 2);

  dump(log);
  for (i = 0; i < 10; i++) {
    debug("Day %i %ld\n", i * 2, timestamp + ONE_DAY * i * 2);
    mu_assert("Done for even day",
	      log_status(log, timestamp + ONE_DAY * i * 2) == 10);
    debug("Day %i %ld\n", i * 2 + 1, timestamp + ONE_DAY * (i * 2 + 1));
    mu_assert("Not done for odd day",
	      log_status(log, timestamp + ONE_DAY * (i * 2 + 1)) == 0);
  }
  log_release(log);
  return NULL;
}

static char *test_skipped_days() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_add(log, 20, timestamp + ONE_DAY);
  mu_assert("Done for second day", log_status(log, timestamp + ONE_DAY) == 10);
  mu_assert("Done for first day", log_status(log, timestamp) == 10);
  log_release(log);
  return NULL;
}

static char *test_skipped_days2() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_add(log, 10, timestamp + ONE_DAY);
  log_add(log, 10, timestamp + ONE_DAY);
  mu_assert("Done for second day", log_status(log, timestamp + ONE_DAY) == 10);
  mu_assert("Done for first day", log_status(log, timestamp) == 10);
  log_release(log);
  return NULL;
}

static char *test_skipped_days3() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_add(log, 10, timestamp + ONE_DAY);
  log_add(log, 20, timestamp + ONE_DAY * 2);
  dump_log(log);
  mu_assert("Done for third day", log_status(log, timestamp + ONE_DAY * 2) == 10);
  mu_assert("Done for second day", log_status(log, timestamp + ONE_DAY) == 10);
  mu_assert("Done for first day", log_status(log, timestamp) == 10);
  log_release(log);
  return NULL;
}

static char *test_two_excercises() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_add(log, 10, timestamp + ONE_DAY);
  log_add(log, 10, timestamp + ONE_DAY + 1);
  mu_assert("Done for second day", log_status(log, timestamp + ONE_DAY) == 10);
  mu_assert("Done for first day", log_status(log, timestamp) == 10);
  log_release(log);
  return NULL;
}

static char *test_serialize() {
  time_t new_stamp = timestamp;
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  size_t result = log_write(log, buf, sizeof(buf));
  expected[0] = '\0';

  mu_assert("Empty log", result == 0 && strcmp(buf, expected) == 0);

  log_add(log, 10, timestamp);
  strftime(expected, sizeof(expected), "%Y.%m.%d\n", localtime(&timestamp));

  result = log_write(log, buf, sizeof(buf));

  mu_assert("One record", result == strlen(expected) && strcmp(buf, expected) == 0);

  new_stamp = timestamp + ONE_DAY;
  log_add(log, 10, new_stamp);
  result = strftime(expected, sizeof(expected), "%Y.%m.%d", localtime(&timestamp));
  strftime(expected+result, sizeof(expected)-result, "-%Y.%m.%d\n", localtime(&new_stamp));

  result = log_write(log, buf, sizeof(buf));
  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);

  mu_assert("Double record", result == strlen(expected) && strcmp(buf, expected) == 0);

  new_stamp = timestamp + ONE_DAY * 3;
  log_add(log, 10, new_stamp);
  strcpy(expected, buf);
  strftime(expected, sizeof(expected), "%Y.%m.%d\n", localtime(&new_stamp));
  strcat(expected, buf);

  result = log_write(log, buf, sizeof(buf));
  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);

  mu_assert("Two records", result == strlen(expected) && strcmp(buf, expected) == 0);

  log_add(log, 10, new_stamp);
  result = strftime(expected, sizeof(expected), "%Y.%m.%d", localtime(&timestamp));
  strftime(expected+result, sizeof(expected)-result, "-%Y.%m.%d\n", localtime(&new_stamp));

  result = log_write(log, buf, sizeof(buf));
  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);

  mu_assert("One record again", result == strlen(expected) && strcmp(buf, expected) == 0);

  log_release(log);
  return NULL;
}

static char *test_read0(maki_uchi_log_t *log) {
  int result;
  strcpy(buf, "");

  result = log_read(log, buf, strlen(buf));

  mu_assert("Read success", result == 0);
  mu_assert("Nothing in log", log_status(log, timestamp) == 0);

  return NULL;
}

static char *test_read1(maki_uchi_log_t *log) {
  int result;
  time_t new_stamp;
  new_stamp = timestamp + ONE_DAY * 2;
  strftime(buf, sizeof(buf), "%Y.%m.%d\n", localtime(&new_stamp));

  result = log_read(log, buf, strlen(buf));

  mu_assert("Read success", result == 0);
  mu_assert("Done for correct day", log_status(log, new_stamp) == 10);
  mu_assert("Not done for day before", log_status(log, new_stamp - ONE_DAY) == 0);
  mu_assert("Not done for day after", log_status(log, new_stamp + ONE_DAY) == 0);
  return NULL;
}

static char *test_read2(maki_uchi_log_t *log) {
  int result;
  time_t new_stamp;

  new_stamp = timestamp + ONE_DAY * 2;
  strftime(buf, sizeof(buf), "%Y.%m.%d", localtime(&timestamp));
  strftime(buf+strlen(buf), sizeof(buf)-strlen(buf), "-%Y.%m.%d\n", localtime(&new_stamp));

  result = log_read(log, buf, strlen(buf));

  printf("Reading [%s] %ld\n", buf, new_stamp);
  dump_log(log);
  mu_assert("Read success", result == 0);
  int i;
  for (i = 0; i < 3; i++)
    mu_assert("Done for correct day", log_status(log, timestamp + i * ONE_DAY) == 10);
  mu_assert("Not done for day after", log_status(log, timestamp + i * ONE_DAY) == 0);
  mu_assert("Not done for two days before", log_status(log, timestamp - ONE_DAY) == 0);
  return NULL;
}

static char *test_read3(maki_uchi_log_t *log) {
  mu_assert("Parse garbage", log_read(log, "asdf", 4) == -1);
  mu_assert("Parse record-garbage", log_read(log, "1970.01.01-asdf", 15) == -1);
  mu_assert("Parse record+garbage+record", log_read(log, "1970.01.01x1970.01.02\n", 21) == -1);
  mu_assert("Parse short", log_read(log, "1970.01.01", 9) == -1);
  return NULL;
}

static char *test_read4(maki_uchi_log_t *log) {
  mu_assert("Parse empty", log_read(log, "1970.01.01-1970.01.02", 0) == 0);
  mu_assert("And it is empty", log_status(log, timestamp) == 0);
  mu_assert("Parse first", log_read(log, "1970.01.01-1970.01.02", 10) == 0);
  mu_assert("First is ok", log_status(log, timestamp) == 10);
  mu_assert("Second is not", log_status(log, timestamp + ONE_DAY) == 0);
  return NULL;
}

static char *test_read() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);

  mu_run_test(test_read0, log);
  mu_run_test(test_read1, log);
  mu_run_test(test_read2, log);
  mu_run_test(test_read3, log);
  mu_run_test(test_read4, log);

  log_release(log);
  return NULL;
}

static char *test_read_file() {
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

static char *test_write_file() {
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
  mu_assert("Write 2 success", result == 11);
  mu_assert("File has correct size", lseek(fd, 0, SEEK_END) == result);
  void *data = mmap(NULL, result, PROT_READ, MAP_PRIVATE, fd, 0);
  mu_assert("mmap success", data != NULL);
  mu_assert("File has correct contents", memcmp(data, "1970.01.01\n", result) == 0);

  log_read(log, "1970.01.01-1970.01.02", 21);
  lseek(fd, 0, SEEK_SET);
  result = log_write_file(log, fd);
  mu_assert("Write 3 success", result == 22);
  mu_assert("File has correct size", lseek(fd, 0, SEEK_END) == result);
  data = mmap(NULL, result, PROT_READ, MAP_PRIVATE, fd, 0);
  mu_assert("mmap success", data != NULL);
  mu_assert("File has correct contents",
	    memcmp(data, "1970.01.01-1970.01.02\n", result) == 0);

  close(fd);
  unlink(filename);

  result = log_write_file(log, fd);
  mu_assert("Write 4 fail", result == -1);

  log_release(log);
  return NULL;
}

static char *test_get_last_entry() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_entry_t *entry;

  entry = log_get_last_entry(log);
  mu_assert("No entries", entry == NULL);

  log_read(log, "1970.01.01", 10);
  entry = log_get_last_entry(log);
  mu_assert("Got one entry", entry != NULL);
  mu_assert("Got correct entry", entry->start == 0);

  log_read(log, "1970.01.03\n1970.01.01", 21);
  entry = log_get_last_entry(log);
  mu_assert("Got one entry", entry != NULL);
  mu_assert("Got correct entry", entry->start == ONE_DAY * 2);

  log_release(log);
  return NULL;
}

static char *test_get_first_entry() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_entry_t *entry;

  entry = log_get_first_entry(log);
  mu_assert("No entries", entry == NULL);

  log_read(log, "1970.01.01", 10);
  entry = log_get_first_entry(log);
  mu_assert("Got one entry", entry != NULL);
  mu_assert("Got correct entry", entry->start == 0);

  log_read(log, "1970.01.03\n1970.01.01", 21);
  entry = log_get_first_entry(log);
  mu_assert("Got one entry", entry != NULL);
  mu_assert("Got correct entry", entry->start == 0);

  log_release(log);
  return NULL;
}

static char *all_tests() {
  mu_run_test(test_log);
  mu_run_test(test_one_day);
  mu_run_test(test_log_multiple_days);
  mu_run_test(test_skipped_days);
  mu_run_test(test_skipped_days2);
  mu_run_test(test_skipped_days3);
  mu_run_test(test_two_excercises);
  mu_run_test(test_serialize);
  mu_run_test(test_read);
  mu_run_test(test_read_file);
  mu_run_test(test_write_file);
  mu_run_test(test_get_last_entry);
  mu_run_test(test_get_first_entry);
  return NULL;
}
 
int main() {
  char *result = all_tests();
  if (result != NULL)
    printf("FAIL: %s\n", result);
  else
    printf("ALL TESTS PASSED\n");
  printf("Tests run: %d\n", tests_run);
  return result != NULL;
}
