#include <alloca.h>
#include <stdio.h>
#include <time.h>

#include "maki-uchi.h"
#include "minunit.h"
 
int tests_run;

#define ONE_DAY (24 * 60 * 60)
 
static char *test_log() {
  time_t timestamp = 0;
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  mu_assert("Nothing for day 0", log_status(log, timestamp) == 0);
  printf("Doing 10 maki-uchi on day 0\n");
  log_add(log, 10, timestamp);
  mu_assert("Done for day 0", log_status(log, timestamp) == 10);
  mu_assert("Not done for day -1", log_status(log, timestamp - ONE_DAY) == 0);
  mu_assert("Not done for day 1", log_status(log, timestamp + ONE_DAY) == 0);
  log_add(log, 10, timestamp + ONE_DAY);
  printf("Doing 10 maki-uchi on day 1\n");
  mu_assert("Done for day 0", log_status(log, timestamp) == 10);
  mu_assert("Not done for day -1", log_status(log, timestamp - ONE_DAY) == 0);
  mu_assert("Done for day 1", log_status(log, timestamp + ONE_DAY) == 10);
  log_release(log);
  return NULL;
}

static char *test_log_multiple_days() {
  time_t timestamp = 0;
  int i;
  maki_uchi_log_t *log = alloca(sizeof(maki_uchi_log_t));
  log_init(log);
  for (i = 0; i < 10; i++)
    log_add(log, 10, timestamp + ONE_DAY * i * 2);

  dump_log(log);
  for (i = 0; i < 10; i++) {
    printf("Day %i %ld\n", i * 2, timestamp + ONE_DAY * i * 2);
    mu_assert("Done for even day",
	      log_status(log, timestamp + ONE_DAY * i * 2) == 10);
    printf("Day %i %ld\n", i * 2 + 1, timestamp + ONE_DAY * (i * 2 + 1));
    mu_assert("Not done for odd day",
	      log_status(log, timestamp + ONE_DAY * (i * 2 + 1)) == 0);
  }
  log_release(log);
  return NULL;
}
 
static char *all_tests() {
  mu_run_test(test_log);
  mu_run_test(test_log_multiple_days);
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
