#ifndef DATE_PARSE_ISO8601_H
#define DATE_PARSE_ISO8601_H
#include <stdint.h>
#include <time.h>

int32_t parse_iso8601(const char *date_string, time_t *date_time);
#endif
