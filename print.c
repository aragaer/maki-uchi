#include <time.h>

#include "print.h"

#define ONE_DAY (60*60*24)

struct tm tm;
static char stamp_buf[11];

char *format_stamp(time_t value) {
  value -= value % ONE_DAY;
  strftime(stamp_buf, sizeof(stamp_buf), "%Y.%m.%d", localtime_r(&value, &tm));
  return stamp_buf;
}

