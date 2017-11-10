#include <stdio.h>
#include <stdlib.h>

#include "entry.h"
#include "list.h"
#include "log.h"

void log_init(maki_uchi_log_t *log) {
  list_init(&log->head);
}

void log_release(maki_uchi_log_t *log) {
  struct list_head *item = log->head.next;
  while (item != &log->head) {
    void *to_free = item;
    item = item->next;
    free(to_free);
  }
  log_init(log);
}

struct log_entry_s *find_entry(maki_uchi_log_t *log, time_t timestamp) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *entry = container_of(item, struct log_entry_s, list);
    if (match(entry, timestamp))
      return entry;
  }
  return NULL;
}

int log_status(maki_uchi_log_t *log, time_t timestamp) {
  struct log_entry_s *entry = find_entry(log, timestamp);
  if (entry == NULL)
    return 0;
  return entry->count;
}

void merge_entries(maki_uchi_log_t *log) {
  struct list_head *first, *second;
  first = log->head.next;
  second = first->next;
  for (; second != &log->head; first = second, second = second->next) {
    struct log_entry_s *first_entry = container_of(first, struct log_entry_s, list);
    struct log_entry_s *second_entry = container_of(second, struct log_entry_s, list);
    if (first_entry->start == second_entry->end + 1 && first_entry->count == second_entry->count) {
      second_entry->end = first_entry->end;
      list_remove_item(first);
      free(first_entry);
    }
  }
}

void insert_entry(maki_uchi_log_t *log, struct log_entry_s *new_entry) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *old_entry = container_of(item, struct log_entry_s, list);
    if (old_entry->end < new_entry->start)
      break;
  }
  list_insert_before(&new_entry->list, item);
  merge_entries(log);
}

void dump_log(maki_uchi_log_t *log) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *entry = container_of(item, struct log_entry_s, list);
    printf("%p: %ld-%ld %d prev=%p next=%p\n", entry, entry->start,
	   entry->end, entry->count, item->prev, item->next);
  }
}

struct log_entry_s *log_get_last_entry(maki_uchi_log_t *log) {
  if (list_is_empty(&log->head))
    return NULL;
  return container_of(log->head.next, struct log_entry_s, list);
}

struct log_entry_s *log_get_first_entry(maki_uchi_log_t *log) {
  if (list_is_empty(&log->head))
    return NULL;
  return container_of(log->head.prev, struct log_entry_s, list);
}

struct log_entry_s *log_get_entry_before(maki_uchi_log_t *log,
					 struct log_entry_s *entry) {
  if (entry == NULL)
    return log_get_last_entry(log);
  struct list_head *next = entry->list.next;
  if (next == &log->head)
    return NULL;
  return container_of(next, struct log_entry_s, list);
}
