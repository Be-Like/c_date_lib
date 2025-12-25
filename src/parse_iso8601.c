#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "date/parse_iso8601.h"

int32_t parse_iso8601(const char *date_string, time_t *date_time) {
  // 2025-01-01T12:00:00.000Z
  char *format = "";
  struct tm t = {0};
  int64_t milliseconds;

  int parse_count =
      sscanf(date_string, format, &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour,
             &t.tm_min, &t.tm_sec, &milliseconds);

  if (parse_count != 7) {
    printf("The provided date (%s) does NOT match the required ISO8601 Date Time format", date_string);
    return 1;
  }

  return 0;
}
