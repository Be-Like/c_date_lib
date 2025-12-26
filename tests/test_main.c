#include <assert.h>
#include <stdio.h>

#include "test_suite.h"

int main(void) {
  printf("Running Date Library Test Suite...\n");
  printf("----------------------------------\n");

  run_date_parse_iso8601_tests();

  printf("----------------------------------\n");

  return 0;
}
