#ifndef _MAKI_UCHI_H_
#define _MAKI_UCHI_H_

#include <time.h>

#include "entry.h"
#include "log.h"

#define DAILY_REQUIREMENT 10
#define ONE_DAY (60*60*24)

void log_add(maki_uchi_log_t *log, int count, time_t timestamp);
size_t log_write(maki_uchi_log_t *log, char *buf, size_t bufsize);
int log_read(maki_uchi_log_t *log, char *buf, size_t buflen);
size_t log_read_file(maki_uchi_log_t *log, int fd);
size_t log_write_file(maki_uchi_log_t *log, int fd);

#endif  // _MAKI_UCHI_H_
