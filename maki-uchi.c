#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maki-uchi.h"

#define ONE_DAY (24 * 60 * 60)

#define container_of(ptr, type, member) ({                      \
      const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
      (type *)( (char *)__mptr - offsetof(type,member) );})

void log_init(maki_uchi_log_t *log) {
  struct list_head *head = &log->head;
  head->next = head->prev = head;
}

void log_release(maki_uchi_log_t *log) {
  struct list_head *item = log->head.next;
  while (item != &log->head) {
    void *to_free = item;
    item = item->next;
    free(to_free);
  }
}

static int match(struct log_entry_s *entry, time_t timestamp) {
  return timestamp >= entry->start && timestamp <= entry->end;
}

static struct log_entry_s *find_entry(maki_uchi_log_t *log, time_t timestamp) {
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

static struct log_entry_s *alloc_entry() {
  void *result = malloc(sizeof(struct log_entry_s));
  if (result == NULL) {
    perror("malloc");
    exit(-1);
  }
  return result;
}

static void insert_before(struct list_head *new, struct list_head *old) {
  new->next = old;
  new->prev = old->prev;
  new->prev->next = new;
  old->prev = new;
}

static void insert_entry(maki_uchi_log_t *log, struct log_entry_s *new_entry) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *old_entry = container_of(item, struct log_entry_s, list);
    if (old_entry->end + 1 == new_entry->start) {
      old_entry->end = new_entry->end;
      return;
    }
    if (old_entry->start == new_entry->end + 1) {
      old_entry->start = new_entry->start;
      return;
    }
    if (old_entry->end < new_entry->start)
      break;
  }
  insert_before(&new_entry->list, item);
}

void log_add(maki_uchi_log_t *log, int count, time_t timestamp) {
  for (; count > 0; timestamp -= ONE_DAY) {
    struct log_entry_s *entry = find_entry(log, timestamp);
    if (entry != NULL)
      continue;
    entry = alloc_entry();
    entry->count = count >= DAILY_REQUIREMENT ? DAILY_REQUIREMENT : 0;
    count -= DAILY_REQUIREMENT;
    entry->start = timestamp - timestamp % ONE_DAY;
    entry->end = entry->start + ONE_DAY - 1;
    insert_entry(log, entry);
  }
}

void dump_log(maki_uchi_log_t *log) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *entry = container_of(item, struct log_entry_s, list);
    printf("%p: %ld-%ld %d prev=%p next=%p\n", entry, entry->start,
	   entry->end, entry->count, item->prev, item->next);
  }
}
