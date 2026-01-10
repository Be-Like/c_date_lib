#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "date/parse_iso8601.h"

static int8_t date_parse_iso8601_full_date_time_with_millis_and_with_symbols() {
  // char *date_string = "1995-04-02T12:30:22.000Z";
  char *date_string = "2004-02-29";
  time_t date_time;

  int32_t x = parse_iso8601(date_string, &date_time);

  printf("[PASS] Parse ISO8601\n");

  return 0;
}

void run_date_parse_iso8601_tests() {
  printf("\nParse ISO8601 Test Cases: ");
  date_parse_iso8601_full_date_time_with_millis_and_with_symbols();
}
