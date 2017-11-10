#ifndef _READER_H_
#define _READER_H_

#include "maki-uchi.h"

typedef enum {
  BEGIN,
  ONE_DATE,
  SECOND_DATE,
  COUNT,
  EOL,
  END,
} reader_state;

struct read_runner {
  char *ptr;
  struct tm *tm;
  struct log_entry_s *entry;
  maki_uchi_log_t *log;
  size_t data_left;
  reader_state state;
};

void reader_step(struct read_runner *);

#endif  // _READER_H_
