#ifndef DATE_PARSE_ISO8601_H
#define DATE_PARSE_ISO8601_H
#include <stdint.h>

#ifdef _WIN32
  #define timegm _mkgmtime
#endif

int32_t parse_iso8601(const char *date_string, int64_t *date_time);
#endif
