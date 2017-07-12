#ifndef _MAKI_UCHI_H_
#define _MAKI_UCHI_H_

#include <time.h>

#define DAILY_REQUIREMENT 10

struct log_entry_s {
  int done;
  time_t timestamp;
  struct log_entry_s *next;
};

typedef struct maki_uchi_log_s {
  struct log_entry_s *entries;
} maki_uchi_log_t;

void log_init(maki_uchi_log_t *log);
int log_status(maki_uchi_log_t *log, time_t timestamp);
void log_add(maki_uchi_log_t *log, int coutn, time_t timestamp);
void log_release(maki_uchi_log_t *log);

#ifdef DEBUG
void dump_log(maki_uchi_log_t *log);
#endif

#endif  // _MAKI_UCHI_H_
