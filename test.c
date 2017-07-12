#include <alloca.h>
#include <stdio.h>
#include <time.h>

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

  dump_log(log);
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
 
static char *all_tests() {
  mu_run_test(test_log);
  mu_run_test(test_one_day);
  mu_run_test(test_log_multiple_days);
  mu_run_test(test_skipped_days);
  mu_run_test(test_skipped_days2);
  mu_run_test(test_skipped_days3);
  mu_run_test(test_two_excercises);
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
