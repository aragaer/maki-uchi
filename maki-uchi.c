#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maki-uchi.h"

#define ONE_DAY (24 * 60 * 60)

void log_init(maki_uchi_log_t *log) {
  log->entries = NULL;
}

void log_release(maki_uchi_log_t *log) {
  struct log_entry_s *entry = log->entries;
  while (entry) {
    void *to_free = entry;
    entry = entry->next;
    free(to_free);
  }
  log->entries = NULL;
}

int log_status(maki_uchi_log_t *log, time_t timestamp) {
  struct log_entry_s *entry;
  for (entry = log->entries; entry; entry = entry->next)
    if (entry->timestamp == timestamp)
      return entry->done;
  return 0;
}

static struct log_entry_s *alloc_entry() {
  void *result = malloc(sizeof(struct log_entry_s));
  if (result == NULL) {
    perror("malloc");
    exit(-1);
  }
  return result;
}

static void insert_entry(maki_uchi_log_t *log, struct log_entry_s *entry) {
  entry->next = log->entries;
  log->entries = entry;
}

void log_add(maki_uchi_log_t *log, int count, time_t timestamp) {
  for (; count > 0; count -= DAILY_REQUIREMENT, timestamp -= ONE_DAY) {
    struct log_entry_s *entry = alloc_entry();
    entry->done = count >= DAILY_REQUIREMENT ? DAILY_REQUIREMENT : 0;
    entry->timestamp = timestamp;
    insert_entry(log, entry);
  }
}

void dump_log(maki_uchi_log_t *log) {
  struct log_entry_s *entry;
  for (entry = log->entries; entry; entry = entry->next)
    printf("%p: %ld %dd %p\n", entry, entry->timestamp,
	   entry->done, entry->next);
}
