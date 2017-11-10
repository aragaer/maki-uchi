#ifndef _LOG_H_
#define _LOG_H_

#include <time.h>

#include "list.h"

typedef struct maki_uchi_log_s {
  struct list_head head;
} maki_uchi_log_t;

void log_init(maki_uchi_log_t *log);
void log_release(maki_uchi_log_t *log);
int log_status(maki_uchi_log_t *log, time_t timestamp);

void merge_entries(maki_uchi_log_t *log);
void insert_entry(maki_uchi_log_t *log, struct log_entry_s *entry);
struct log_entry_s *find_entry(maki_uchi_log_t *log, time_t timestamp);

log_entry_t *log_get_last_entry(maki_uchi_log_t *log);
log_entry_t *log_get_first_entry(maki_uchi_log_t *log);
log_entry_t *log_get_entry_before(maki_uchi_log_t *log, log_entry_t *entry);

#ifdef DEBUG
void dump_log(maki_uchi_log_t *log);
#endif

#endif  // _LOG_H_
