#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "date/parse_iso8601.h"

static int8_t date_parse_iso8601_test() {
  char *date_string = "25-1-1T06:05:02.371Z";
  time_t date_time;

  parse_iso8601(date_string, &date_time);

  printf("[PASS] Parse ISO8601\n");

  return 0;
}

void run_date_parse_iso8601_tests() {
  printf("\nParse ISO8601 Test Cases: ");
  date_parse_iso8601_test();
}
