#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maki-uchi.h"

void log_init(maki_uchi_log_t *log) {
  log->entries_head = NULL;
}

int log_status(maki_uchi_log_t *log, time_t timestamp) {
  struct log_entry_s *entry;
  for (entry = log->entries_head; entry; entry = entry->next)
    if (entry->timestamp == timestamp)
      return entry->done;
  return 0;
}

void log_add(maki_uchi_log_t *log, int count, time_t timestamp) {
  struct log_entry_s *entry = malloc(sizeof(*entry));
  if (entry == NULL) {
    perror("malloc");
    exit(-1);
  }
  entry->done = count;
  entry->timestamp = timestamp;
  entry->next = log->entries_head;
  log->entries_head = entry;
}

void dump_log(maki_uchi_log_t *log) {
  struct log_entry_s *entry;
  for (entry = log->entries_head; entry; entry = entry->next)
    printf("%p: %ld %dd %p\n", entry, entry->timestamp,
	   entry->done, entry->next);
}
