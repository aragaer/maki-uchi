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
    struct log_entry_s *entry = from_list_head(item);
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
  struct list_head *first;
  struct list_head *second;
  first = log->head.next;
  second = first->next;
  for (; second != &log->head; first = second, second = second->next) {
    struct log_entry_s *first_entry = from_list_head(first);
    struct log_entry_s *second_entry = from_list_head(second);
    if (first_entry->start == second_entry->end + 1 && first_entry->count == second_entry->count) {
      second_entry->end = first_entry->end;
      list_remove_item(first);
      free(first_entry);
    }
  }
}

void insert_entry(maki_uchi_log_t *log, struct log_entry_s *new_entry) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next)
    if (from_list_head(item)->end < new_entry->start)
      break;
  list_insert_before(&new_entry->list, item);
  merge_entries(log);
}

static void dump_entry(struct log_entry_s *entry, struct list_head *item) {
  printf("%p: %ld-%ld %d prev=%p next=%p\n", entry, entry->start,
	 entry->end, entry->count, item->prev, item->next);
}

void dump_log(maki_uchi_log_t *log) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next)
    dump_entry(from_list_head(item), item);
}

struct log_entry_s *log_get_last_entry(maki_uchi_log_t *log) {
  if (list_is_empty(&log->head))
    return NULL;
  return from_list_head(log->head.next);
}

struct log_entry_s *log_get_first_entry(maki_uchi_log_t *log) {
  if (list_is_empty(&log->head))
    return NULL;
  return from_list_head(log->head.prev);
}

struct log_entry_s *log_get_entry_before(maki_uchi_log_t *log,
					 struct log_entry_s *entry) {
  if (entry == NULL)
    return log_get_last_entry(log);
  struct list_head *next = entry->list.next;
  if (next == &log->head)
    return NULL;
  return from_list_head(next);
}
