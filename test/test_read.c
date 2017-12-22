#include <alloca.h>
#include <stdio.h>
#include <string.h>

#include "maki-uchi.h"
#include "minunit.h"

#define ONE_DAY (24 * 60 * 60)

static time_t timestamp = 0;
static char buf[1024];

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

static char *test_read5(maki_uchi_log_t *log) {
  int result;
  time_t new_stamp;
  new_stamp = timestamp + ONE_DAY * 2;
  strftime(buf, sizeof(buf), "%Y.%m.%d 9\n", localtime(&new_stamp));

  result = log_read(log, buf, strlen(buf));

  mu_assert("Read success", result == 0);
  mu_assert("Done for correct day", log_status(log, new_stamp) == 9);
  mu_assert("Not done for day before", log_status(log, new_stamp - ONE_DAY) == 0);
  mu_assert("Not done for day after", log_status(log, new_stamp + ONE_DAY) == 0);
  return NULL;
}

static char *test_read6(maki_uchi_log_t *log) {
  int result;
  time_t new_stamp;

  new_stamp = timestamp + ONE_DAY * 2;
  strftime(buf, sizeof(buf), "%Y.%m.%d", localtime(&timestamp));
  strftime(buf+strlen(buf), sizeof(buf)-strlen(buf), "-%Y.%m.%d 8\n", localtime(&new_stamp));

  result = log_read(log, buf, strlen(buf));

  printf("Reading [%s] %ld\n", buf, new_stamp);
  dump_log(log);
  mu_assert("Read success", result == 0);
  int i;
  for (i = 0; i < 3; i++)
    mu_assert("Done for correct day", log_status(log, timestamp + i * ONE_DAY) == 8);
  mu_assert("Not done for day after", log_status(log, timestamp + i * ONE_DAY) == 0);
  mu_assert("Not done for two days before", log_status(log, timestamp - ONE_DAY) == 0);
  return NULL;
}

char *test_read() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);

  mu_run_test(test_read0, log);
  mu_run_test(test_read1, log);
  mu_run_test(test_read2, log);
  mu_run_test(test_read3, log);
  mu_run_test(test_read4, log);
  mu_run_test(test_read5, log);
  mu_run_test(test_read6, log);

  log_release(log);
  return NULL;
}
