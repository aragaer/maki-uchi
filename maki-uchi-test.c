#include <stdio.h>
#include "minunit.h"
 
int tests_run;
 
static char *test_foo() {
  mu_assert("error, 7 != 7", 7 == 7);
  return 0;
}
 
static char *all_tests() {
  mu_run_test(test_foo);
  return 0;
}
 
int main(int argc, char *argv[]) {
  char *result = all_tests();
  if (result != NULL)
    printf("FAIL: %s\n", result);
  else
    printf("ALL TESTS PASSED\n");
  printf("Tests run: %d\n", tests_run);
  return result != NULL;
}
