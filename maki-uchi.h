#ifndef _MAKI_UCHI_H_
#define _MAKI_UCHI_H_

#include <time.h>

#define DAILY_REQUIREMENT 10

struct list_head {
  struct list_head *prev, *next;
};

typedef struct log_entry_s {
  struct list_head list;
  int count;
  time_t start, end;
} log_entry_t;

typedef struct maki_uchi_log_s {
  struct list_head head;
} maki_uchi_log_t;

void log_init(maki_uchi_log_t *log);
int log_status(maki_uchi_log_t *log, time_t timestamp);
void log_add(maki_uchi_log_t *log, int coutn, time_t timestamp);
void log_release(maki_uchi_log_t *log);
size_t log_write(maki_uchi_log_t *log, char *buf, size_t bufsize);
int log_read(maki_uchi_log_t *log, char *buf, size_t buflen);
size_t log_read_file(maki_uchi_log_t *log, int fd);
size_t log_write_file(maki_uchi_log_t *log, int fd);
struct log_entry_s *log_get_last_entry(maki_uchi_log_t *log);

#ifdef DEBUG
void dump_log(maki_uchi_log_t *log);
#endif

#endif  // _MAKI_UCHI_H_
