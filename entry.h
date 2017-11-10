#ifndef _ENTRY_H_
#define _ENTRY_H_

#include "list.h"

typedef struct log_entry_s {
  struct list_head list;
  int count;
  time_t start, end;
} log_entry_t;

struct log_entry_s *alloc_entry();
struct log_entry_s *create_entry(time_t start, time_t end, int count);
int match(struct log_entry_s *entry, time_t timestamp);


#endif  // _ENTRY_H_
