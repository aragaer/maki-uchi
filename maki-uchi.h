#ifndef _MAKI_UCHI_H_
#define _MAKI_UCHI_H_

#include <time.h>

#define DAILY_REQUIREMENT 10

struct log_entry_s {
  int count;
  time_t start, end;
  struct log_entry_s *prev, *next;
};

typedef struct maki_uchi_log_s {
  struct log_entry_s head;
} maki_uchi_log_t;

void log_init(maki_uchi_log_t *log);
int log_status(maki_uchi_log_t *log, time_t timestamp);
void log_add(maki_uchi_log_t *log, int coutn, time_t timestamp);
void log_release(maki_uchi_log_t *log);

#ifdef DEBUG
void dump_log(maki_uchi_log_t *log);
#endif

#endif  // _MAKI_UCHI_H_
