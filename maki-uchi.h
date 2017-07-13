#ifndef _MAKI_UCHI_H_
#define _MAKI_UCHI_H_

#include <time.h>

#define DAILY_REQUIREMENT 10

struct list_head {
  struct list_head *prev, *next;
};

struct log_entry_s {
  struct list_head list;
  int count;
  time_t start, end;
};

typedef struct maki_uchi_log_s {
  struct list_head head;
} maki_uchi_log_t;

void log_init(maki_uchi_log_t *log);
int log_status(maki_uchi_log_t *log, time_t timestamp);
void log_add(maki_uchi_log_t *log, int coutn, time_t timestamp);
void log_release(maki_uchi_log_t *log);
size_t log_write(maki_uchi_log_t *log, char *buf, size_t bufsize);

#ifdef DEBUG
void dump_log(maki_uchi_log_t *log);
#endif

#endif  // _MAKI_UCHI_H_
