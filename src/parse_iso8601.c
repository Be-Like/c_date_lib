#define _DEFAULT_SOURCE

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "date/parse_iso8601.h"

static int8_t parse_n_digits(const char *ptr, int n, int *out) {
  int i = 0;
  int value = 0;

  while (i < n) {
    if (isdigit((unsigned char)ptr[i])) {
      value = 10 * value + (ptr[i] - '0');
      i++;
    } else {
      return 1;
    }
  }

  *out = value;
  return 0;
}

static time_t date_mktime(const struct tm *t) {
  long year = t->tm_year + 1900;
  long month = t->tm_mon;

  if (month <= 1) {
    year -= 1;
    month += 12;
  }
  printf("\nMonth: %ld\n", month);

  long days = (365 * year) + (year / 4) - (year / 100) + (year / 400);
  printf("\nDays: %ld\n", days);
  days += (306001 * (month + 1)) / 10000;
  printf("\nDays: %ld\n", days);
  days += t->tm_mday;
  printf("\nDays: %ld\n", days);

  long days_since_epoch = days - 719591;

  time_t seconds = days_since_epoch * 86400;
  seconds += t->tm_hour * 3600;
  seconds += t->tm_min * 60;
  seconds += t->tm_sec;

  return seconds;
}

static void printTm(struct tm t) {
  time_t x = mktime(&t);
  printf("\nX: %ld\n", x);
  printf("The GM time is: %s\n", asctime(gmtime(&x)));
  printf("The local time is: %s\n", asctime(localtime(&x)));
}

struct iso8601_sequence_t {
  int length;
  int uses_symbols;
  int date_start_index;
  int date_end_index;
  int timestamp_start_index;
  int timestamp_end_index;
  int is_utc;
  int utc_offset_start_index;
  int utc_offset_end_index;
};

static int32_t sequence_iso8601(const char *date_string,
                                struct iso8601_sequence_t *t) {
  // Initialize the sequence state
  t->length = 0;
  t->uses_symbols = -1;
  t->date_start_index = 0;
  t->date_end_index = -1;
  t->timestamp_start_index = -1;
  t->timestamp_end_index = -1;
  t->is_utc = -1;
  t->utc_offset_start_index = -1;
  t->utc_offset_end_index = -1;

  int i = 0;

  while (date_string[i] != '\0') {
    // Evaluating date
    if (t->date_end_index == -1) {
      if (date_string[i] == '-' && t->uses_symbols == -1) {
        t->uses_symbols = 1;
      } else if (date_string[i] == 'T') {
        if (t->uses_symbols == -1) {
          t->uses_symbols = 0;
        }
        t->date_end_index = i - 1;
        t->timestamp_start_index = i + 1;
      }
    }
    // Evaluating timestamp
    else if (t->date_end_index > 0 && t->timestamp_start_index != -1 &&
             t->timestamp_end_index == -1) {
      if (date_string[i] == ':' && t->uses_symbols == 0) {
        return 1; // Invalid date time sequence
      } else if (date_string[i] == 'Z' || date_string[i] == 'z') {
        t->is_utc = 1;
        t->timestamp_end_index = i - 1;
      } else if (date_string[i] == '+' || date_string[i] == '-') {
        if (t->is_utc != -1) {
          return 1; // Invalid date time sequence
        }

        t->is_utc = 0;
        t->utc_offset_start_index = i + 1;
      }
    }
    // Evaluating time zone
    else if (t->date_end_index > 0 && t->timestamp_start_index > 0 &&
             t->timestamp_end_index > 0 && t->is_utc == 0 &&
             t->utc_offset_start_index != -1 && t->utc_offset_end_index == -1) {
      if (date_string[i] == ':' && t->uses_symbols == 0) {
        return 1; // Invalid date time sequence
      }
    }

    i++;
  }

  t->length = i;

  if (t->uses_symbols == -1) {
    t->uses_symbols = 0;
  }

  // Ensure sequence termination
  if (t->date_end_index == -1) {
    t->date_end_index = i - 1;
  } else if (t->date_end_index != -1 && t->timestamp_start_index != -1 &&
             t->timestamp_end_index == -1) {
    t->timestamp_end_index = i - 1;
  } else if (t->date_end_index != -1 && t->timestamp_start_index != -1 &&
             t->timestamp_end_index != -1 && t->is_utc == 0 &&
             t->utc_offset_start_index != -1 && t->utc_offset_end_index == -1) {
    t->utc_offset_end_index = i - 1;
  }

  return 0;
}

int compute_days_in_month(const int year, const int month) {
  int month_days[12] = {
      31, // January
      0,  // February
      31, // March
      30, // April
      31, // May
      30, // June
      31, // July
      31, // August
      30, // September
      31, // October
      30, // November
      31  // December
  };
  if (month == 1) {
    // Determine whether it is a leap year
    int evaluating_year = year + 1900;
    if (evaluating_year % 4 == 0) {
      if (evaluating_year % 100 == 0 && evaluating_year % 400 != 0) {
        month_days[1] = 28;
      } else {
        month_days[1] = 29;
      }
    } else {
      month_days[1] = 28;
    }
  }

  return month_days[month];
}

int32_t parse_date(const char *date, const int length, const int symbols,
                   struct tm *t) {
  if (length == 2) {
    if (parse_n_digits(date, length, &t->tm_year) == 0) {
      t->tm_year = (t->tm_year * 100) - 1900;
      return 0;
    } else {
      return 1; // Invalid date format.
    }
  }

  if (length == 3) {
    if (parse_n_digits(date, length, &t->tm_year) == 0) {
      t->tm_year = (t->tm_year * 10) - 1900;
      return 0;
    } else {
      return 1; // Invalid date format.
    }
  }

  if (length == 4) {
    if (parse_n_digits(date, length, &t->tm_year) == 0) {
      t->tm_year = t->tm_year - 1900;
      return 0;
    } else {
      return 1;
    }
  }

  if (4 < length && length < 7) {
    return 1; // Invalid date format.
  }

  int current_parse_index = 0;
  const int YEAR_DIGIT_COUNT = 4;
  if (parse_n_digits(&date[current_parse_index], YEAR_DIGIT_COUNT,
                     &t->tm_year) == 0) {
    t->tm_year -= 1900;
    current_parse_index += YEAR_DIGIT_COUNT;
  } else {
    return 1; // Invalid date format.
  }

  if (symbols == 0) {
    /*
     * YYYYMM (invalid)
     * YYYYDDD (not yet supported)
     * YYYYWww (not yet supported)
     * YYYYMMDD
     */
    if (length == 7) {
      /*
       * Dates to be supported here
       * * YYYYDDD
       * * YYYYWww
       */
      return 2; // Not yet supported.
    } else if (length == 8) {
      /*
       * Dates to be supported here
       * * YYYYMMDD
       * * YYYYWwwD (not yet supported)
       */

      // Parse the month
      if (parse_n_digits(&date[current_parse_index], 2, &t->tm_mon) == 0) {
        t->tm_mon--;

        if (t->tm_mon < 0 || t->tm_mon > 11) {
          return 1; // Invalid date format.
        } else {
          printf("\nMonth evaluation: %i\n", t->tm_mon);
          current_parse_index += 2;
        }
      } else {
        if (date[current_parse_index] == 'W' ||
            date[current_parse_index] == 'w') {
          return 2; // Date format not yet supported.
        } else {
          return 1; // Invalid date format.
        }
      }

      // Parse the month date
      if (parse_n_digits(&date[current_parse_index], 2, &t->tm_mday) == 0) {
        if (1 <= t->tm_mday && t->tm_mday <= compute_days_in_month(t->tm_year, t->tm_mon)) {
          current_parse_index += 2;
        } else {
          return 1; // Invalid date format.
        }
      } else {
        return 1; // Invalid date format.
      }
    } else {
      return 1; // Invalid date format.
    }
  } else {
    /*
     * YYYY-MM
     * YYYY-DDD (not yet supported)
     * YYYY-Www (not yet supported)
     * YYYY-MM-DD
     */
    if (length == 7) {
      /*
       * Dates to be supported here
       * * YYYY-MM
       */

      // Ensure the symbol is present.
      if (date[current_parse_index] == '-') {
        current_parse_index++;
      } else {
        return 1; // Invalid date format.
      }

      // Parse the month
      if (parse_n_digits(&date[current_parse_index], 2, &t->tm_mon) == 0) {
        t->tm_mon--;
        current_parse_index += 2;
      } else {
        return 1; // Invalid date format.
      }
    } else if (length == 8) {
      /*
       * Dates to be supported here
       * * YYYY-DDD
       * * YYYY-Www
       */
      return 2; // Not yet supported.
    } else if (length == 9) {
      return 1; // Invalid date format.
    } else if (length == 10) {
      /*
       * Dates to be supported here
       * * YYYY-MM-DD
       */

      // Ensure the symbol is present
      if (date[current_parse_index] == '-') {
        current_parse_index++;
      } else {
        return 1; // Invalid date format.
      }

      // Parse the month
      if (parse_n_digits(&date[current_parse_index], 2, &t->tm_mon) == 0) {
        t->tm_mon--;
        current_parse_index += 2;
      } else {
        return 1; // Invalid date format.
      }

      // Ensure the symbol is present
      if (date[current_parse_index] == '-') {
        current_parse_index++;
      } else {
        return 1; // Invalid date format.
      }

      // Parse the month date
      if (parse_n_digits(&date[current_parse_index], 2, &t->tm_mday) == 0) {
        if (1 <= t->tm_mday && t->tm_mday <= compute_days_in_month(t->tm_year, t->tm_mon)) {
          current_parse_index += 2;
        } else {
          return 1; // Invalid date format.
        }
      } else {
        return 1; // Invalid date format.
      }
    } else {
      return 1; // Invalid date format.
    }
  }

  if (current_parse_index != length) {
    return 1; // Invalid date format.
  }

  return 0;
}

int32_t parse_timestamp(const char *timestamp, const int length, struct tm *t) {
  char ss[20];

  strncpy(ss, timestamp + 0, length);
  ss[length] = '\0';

  printf("\nWill parse the timestamp: %s\n", ss);

  return 0;
}

int32_t parse_time_zone() {
  printf("\nWill parse the time zone\n");

  return 0;
}

int32_t parse_iso8601(const char *date_string, int64_t *date_time) {
  struct iso8601_sequence_t seq;
  int32_t sequencing_status = sequence_iso8601(date_string, &seq);

  if (sequencing_status == 1) {
    return 1; // Invalid date string format.
  } else {
    struct tm t = {
        0,    // seconds
        0,    // minutes
        0,    // hours
        1,    // month date (default to the 1st)
        0,    // month (default to January)
        0,    // year
        0,    // weekday
        0,    // year day
        -1,   // daylight savings time (default to "not sure")
        0,    // gmt offset (offset from UTC) - NOT SUPPORTED BY Mktime
        "UTC" // time zone - NOT SUPPORTED BY mktime
    };

    if (seq.length < 2) {
      return 1; // Invalid date string format.
    }

    int32_t date_parse_status = parse_date(
        date_string + seq.date_start_index,
        seq.date_end_index - seq.date_start_index + 1, seq.uses_symbols, &t);

    if (date_parse_status == 1) {
      return 1; // Invalid date string format.
    } else if (date_parse_status == 2) {
      return 2; // Date not yet supported.
    }
    int32_t timestamp_parse_status = parse_timestamp(
        date_string + seq.timestamp_start_index,
        seq.timestamp_end_index - seq.timestamp_start_index + 1, &t);

    printf("\nTM struct:\n");
    printf("\tyear: %i\n", t.tm_year);
    printf("\tmonth: %i\n", t.tm_mon);
    printf("\tday: %i\n", t.tm_mday);
    printf("\tday of the year: %i\n", t.tm_yday);
    printf("\thour: %i\n", t.tm_hour);
    printf("\tminutes: %i\n", t.tm_min);
    printf("\tseconds: %i\n", t.tm_sec);

    // printf("\nDate make time: %ld\n", date_mktime(&t));
  }

  return 0;
}
// int32_t parse_iso8601(const char *date_string, int64_t *date_time) {
//   /*
//    * Process:
//    * * split the calendar date from the time
//    * * evaluate the year (e.g. YYYY-MM-DD or YYYYMMDD)
//    *   * YYYY-MM-DD or YYYYMMDD
//    *   * YYYY-DDD or YYYYDDD (optional)
//    *   * YYYY-Www-D or YYYYWwwD (optional)
//    *   * YYYY-Www or YYYYWww (optional)
//    * * evaluate the time portion
//    *   * Thh:mm:ss.sss or Thhmmss.sss
//    *   * Thh:mm:ss or Thhmmss
//    *   * Thh:mm or Thhmm
//    *   * Thh
//    *   * Thh:mm.mmm or Thhmm.mmm (optional)
//    *   * Thh.hhh (optional)
//    * * evaluate the time zone
//    *   * <time>Z
//    *   * <time>+/-hh:mm
//    *   * <time>+/-hhmm
//    *   * <time>+/-hh
//    */

//   int current_parse_index = 0;

//
//   if (date_string[current_parse_index] == '-') {
//     dts.contains_symbols = 1;
//     current_parse_index++;
//   }
//
//   if (date_string[current_parse_index] == 'W' ||
//       date_string[current_parse_index] == 'w') {
//     return 2; // Week dates format is not supported at this time (WIP).
//   }
//
//   if (dts.contains_symbols) {
//     if (current_parse_index + 2 <= date_string_length) {
//       if (date_string[current_parse_index + 2] == '\0') {
//         // it is the end of the line ("YYYY-MM")
//         printf("End of the line %c", date_string[current_parse_index + 2]);
//       } else if (isdigit(date_string[current_parse_index + 2])) {
//         // it is an ordinal date ()
//
//       } else {
//         // Then it is a standard dates
//       }
//       if (date_string[current_parse_index + 2] == '-') {
//       } else {
//       }
//     }
//   } else {
//     // do this logic here
//   }
//
//   if (date_string[current_parse_index + 2] <= date_string_length &&
//       date_string[current_parse_index + 2] == '-') {
//     if (parse_n_digits(&date_string[current_parse_index], 2, &t.tm_mon) == 0)
//     {
//       printf("\nChecking on the date string: %s\n", date_string);
//       printf("Month evaluation: %i\n", t.tm_mon);
//       current_parse_index += 2;
//     } else {
//       return 1;
//     }
//   }
