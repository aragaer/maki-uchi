#include <stdio.h>
#include <stdlib.h>

#include "entry.h"

struct log_entry_s *alloc_entry() {
  void *result = malloc(sizeof(struct log_entry_s));
  if (result == NULL) {
    perror("malloc");
    exit(-1);
  }
  return result;
}

struct log_entry_s *create_entry(time_t start, time_t end, int count) {
  struct log_entry_s *result = alloc_entry();
  result->start = start;
  result->end = end;
  result->count = count;
  return result;
}

int match(struct log_entry_s *entry, time_t timestamp) {
  return timestamp >= entry->start && timestamp <= entry->end;
}

struct log_entry_s *from_list_head(struct list_head *item) {
  return container_of(item, struct log_entry_s, list);
}
