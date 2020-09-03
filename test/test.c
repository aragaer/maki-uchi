#define _XOPEN_SOURCE 500
#include <alloca.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "maki-uchi.h"
#include "minunit.h"
#include "test.h"

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
  strftime(expected, sizeof(expected), "%Y.%m.%d 10\n", localtime(&timestamp));

  result = log_write(log, buf, sizeof(buf));

  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);
  mu_assert("One record", result == strlen(expected) && strcmp(buf, expected) == 0);

  new_stamp = timestamp + ONE_DAY;
  log_add(log, 10, new_stamp);
  result = strftime(expected, sizeof(expected), "%Y.%m.%d", localtime(&timestamp));
  strftime(expected+result, sizeof(expected)-result, "-%Y.%m.%d 10\n", localtime(&new_stamp));

  result = log_write(log, buf, sizeof(buf));
  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);

  mu_assert("Double record", result == strlen(expected) && strcmp(buf, expected) == 0);

  new_stamp = timestamp + ONE_DAY * 3;
  log_add(log, 10, new_stamp);
  strcpy(expected, buf);
  strftime(expected, sizeof(expected), "%Y.%m.%d 10\n", localtime(&new_stamp));
  strcat(expected, buf);

  result = log_write(log, buf, sizeof(buf));
  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);

  mu_assert("Two records", result == strlen(expected) && strcmp(buf, expected) == 0);

  log_add(log, 10, new_stamp);
  result = strftime(expected, sizeof(expected), "%Y.%m.%d", localtime(&timestamp));
  strftime(expected+result, sizeof(expected)-result, "-%Y.%m.%d 10\n", localtime(&new_stamp));

  result = log_write(log, buf, sizeof(buf));
  debug("Expecting:\n===\n%s===\nGot:\n===\n%s===\n", expected, buf);

  mu_assert("One record again", result == strlen(expected) && strcmp(buf, expected) == 0);

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

static char *test_get_entry_before() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_entry_t *entry;

  entry = log_get_entry_before(log, NULL);
  mu_assert("Nothing before NULL", entry == NULL);

  log_read(log, "1970.01.01", 10);
  entry = log_get_entry_before(log, NULL);
  mu_assert("Latest entry is before NULL", entry == log_get_last_entry(log));
  mu_assert("Nothing before it", log_get_entry_before(log, entry) == NULL);

  log_read(log, "1970.01.03\n1970.01.01", 21);
  entry = log_get_entry_before(log, NULL);
  mu_assert("Latest entry is before NULL", entry == log_get_last_entry(log));
  entry = log_get_entry_before(log, entry);
  mu_assert("One more before it", entry == log_get_first_entry(log));
  entry = log_get_entry_before(log, entry);
  mu_assert("Nothing more", entry == NULL);

  log_release(log);
  return NULL;
}

static char *test_log_incomplete() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  mu_assert("Nothing for day 0", log_status(log, timestamp) == 0);
  debug("Doing 7 maki-uchi on day 0\n");
  log_add(log, 7, timestamp);
  mu_assert("Just 7 for day 0", log_status(log, timestamp) == 7);
  mu_assert("Not done for day -1", log_status(log, timestamp - ONE_DAY) == 0);
  mu_assert("Not done for day 1", log_status(log, timestamp + ONE_DAY) == 0);
  log_add(log, 10, timestamp + ONE_DAY);
  debug("Doing 10 maki-uchi on day 1\n");
  mu_assert("Just 7 for day 0", log_status(log, timestamp) == 7);
  mu_assert("Not done for day -1", log_status(log, timestamp - ONE_DAY) == 0);
  mu_assert("Done for day 1", log_status(log, timestamp + ONE_DAY) == 10);
  log_add(log, 5, timestamp + ONE_DAY * 2);
  log_add(log, 11, timestamp + ONE_DAY * 3);
  mu_assert("Just 6 for day 2", log_status(log, timestamp + ONE_DAY * 2) == 6);
  log_add(log, 17, timestamp + ONE_DAY * 4);
  int i;
  for (i = 0; i < 4; i++)
    mu_assert("Done", log_status(log, timestamp + ONE_DAY * i) == 10);
  log_release(log);
  return NULL;
}

static char *test_log_incomplete_split() {
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  log_add(log, 5, timestamp);
  log_add(log, 5, timestamp + ONE_DAY);
  mu_assert("Just 5 for day 0", log_status(log, timestamp) == 5);
  mu_assert("Just 5 for day 1", log_status(log, timestamp + ONE_DAY) == 5);
  log_add(log, 11, timestamp + ONE_DAY * 2);
  mu_assert("Just 5 for day 0", log_status(log, timestamp) == 5);
  mu_assert("Just 6 for day 1", log_status(log, timestamp + ONE_DAY) == 6);
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
  mu_run_test(test_get_entry_before);
  mu_run_test(test_log_incomplete);
  mu_run_test(test_log_incomplete_split);
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
