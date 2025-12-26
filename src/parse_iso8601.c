#define _DEFAULT_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "date/parse_iso8601.h"

int32_t parse_iso8601(const char *date_string, int64_t *date_time) {
  /*
   * Process:
   * * split the calendar date from the time
   * * evaluate the year (e.g. YYYY-MM-DD or YYYYMMDD)
   *   * YYYY-MM-DD or YYYYMMDD
   *   * YYYY-Www-D or YYYYWwwD (optional)
   *   * YYYY-Www or YYYYWww (optional)
   *   * YYYY-DDD or YYYYDDD (optional)
   * * evaluate the time portion
   *   * Thh:mm:ss.sss or Thhmmss.sss
   *   * Thh:mm:ss or Thhmmss
   *   * Thh:mm or Thhmm
   *   * Thh
   *   * Thh:mm.mmm or Thhmm.mmm (optional)
   *   * Thh.hhh (optional)
   */

  char *format = "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ";
  struct tm t = {0};
  int milliseconds;

  int parse_count =
      sscanf(date_string, format, &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour,
             &t.tm_min, &t.tm_sec, &milliseconds);

  if (parse_count != 7) {
    printf("\nThe provided date (%s) does NOT match the required ISO8601 Date "
           "Time format.\n",
           date_string);
    return 1;
  }

  // Modify the year and month offsets.
  t.tm_year -= 1900;
  t.tm_mon -= 1;

  int64_t seconds_since_epoch = timegm(&t);
  *date_time = ((int64_t)seconds_since_epoch * 1000) + milliseconds;;

  return 0;
}
