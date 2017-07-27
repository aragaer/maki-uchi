#define _XOPEN_SOURCE 500
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

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
  log_init(log);
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

static void remove_entry(struct list_head *item) {
  struct list_head *next = item->next;
  next->prev = item->prev;
  next->prev->next = next;
}

static void merge_entries(maki_uchi_log_t *log) {
  struct list_head *first, *second;
  first = log->head.next;
  second = first->next;
  for (; second != &log->head; first = second, second = second->next) {
    struct log_entry_s *first_entry = container_of(first, struct log_entry_s, list);
    struct log_entry_s *second_entry = container_of(second, struct log_entry_s, list);
    if (first_entry->start == second_entry->end + 1) {
      second_entry->end = first_entry->end;
      remove_entry(first);
      free(first_entry);
    }
  }
}

static void insert_entry(maki_uchi_log_t *log, struct log_entry_s *new_entry) {
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *old_entry = container_of(item, struct log_entry_s, list);
    if (old_entry->end < new_entry->start)
      break;
  }
  insert_before(&new_entry->list, item);
  merge_entries(log);
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

size_t log_write(maki_uchi_log_t *log, char *buf, size_t bufsize) {
  struct tm tm;
  struct list_head *item;
  size_t result = 0;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *entry = container_of(item, struct log_entry_s, list);
    localtime_r(&entry->start, &tm);
    result += strftime(buf+result, bufsize-result, "%Y.%m.%d", &tm);
    if (entry->end - entry->start > ONE_DAY) {
      time_t last_day = entry->end - ONE_DAY + 1;
      localtime_r(&last_day, &tm);
      result += strftime(buf+result, bufsize-result, "-%Y.%m.%d", &tm);
    }
    if (result < bufsize) {
      buf[result++] = '\n';
      buf[result] = '\0';
    }
  }
  return result;
}

typedef enum {
  BEGIN,
  ONE_DATE,
  SECOND_DATE,
  EOL,
  END,
  ERROR,
} reader_state;

struct read_runner {
  char *ptr;
  struct tm *tm;
  struct log_entry_s *entry;
  maki_uchi_log_t *log;
  size_t data_left;
  reader_state state;
};

#define DATE_LEN 10

static reader_state process_date(struct read_runner *this) {
  if (this->data_left < DATE_LEN
      || strptime(this->ptr, "%Y.%m.%d", this->tm) == NULL) {
    return ERROR;
  }
  this->ptr += DATE_LEN;
  this->data_left -= DATE_LEN;
  if (this->state == BEGIN) {
    this->entry = alloc_entry();
    this->entry->start = mktime(this->tm);
    return ONE_DATE;
  } else
    return EOL;
}

static reader_state process_hyphen(struct read_runner *this) {
  if (this->data_left == 0 || *this->ptr != '-')
    return EOL;
  this->ptr++;
  this->data_left--;
  return SECOND_DATE;
}

static reader_state process_eol(struct read_runner *this) {
  this->entry->end = mktime(this->tm) + ONE_DAY - 1;
  this->entry->count = DAILY_REQUIREMENT;
  insert_entry(this->log, this->entry);
  this->entry = NULL;
  if (this->data_left <= 0)
    return END;
  else if (*this->ptr != '\n')
    return ERROR;
  this->ptr++;
  this->data_left--;
  return BEGIN;
}

reader_state (*sm[6])(struct read_runner *) = {
  [BEGIN] = process_date,
  [ONE_DATE] = process_hyphen,
  [SECOND_DATE] = process_date,
  [EOL] = process_eol,
};

static reader_state step(struct read_runner *this) {
  switch (this->state) {
  case BEGIN:
  case SECOND_DATE:
    return process_date(this);
  case ONE_DATE:
    return process_hyphen(this);
  case EOL:
    return process_eol(this);
  default:
    break;
  }
  return this->state;
}

int log_read(maki_uchi_log_t *log, char *buf, size_t buflen) {
  time_t base = 0;
  struct tm tm;
  localtime_r(&base, &tm);
  log_release(log);
  struct read_runner reader = {
    .ptr = buf,
    .tm = &tm,
    .data_left = buflen,
    .state = BEGIN,
    .entry = NULL,
    .log = log,
  };
  while (reader.state != ERROR && reader.state != END)
    reader.state = step(&reader);
  free(reader.entry);
  if (reader.data_left == 0 || *reader.ptr == '\0')
    return 0;
  else
    return -1;
}

size_t log_read_file(maki_uchi_log_t *log, int fd) {
  off_t len = lseek(fd, 0, SEEK_END);
  log_release(log);
  if (len == 0)
    return 0;
  void *data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == MAP_FAILED)
    return -1;
  int result = log_read(log, data, len);
  munmap(data, len);
  return result == 0 ? len : -1;
}

size_t log_write_file(maki_uchi_log_t *log, int fd) {
  size_t file_size = 0;
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next) {
    struct log_entry_s *entry = container_of(item, struct log_entry_s, list);
    if (entry->end == entry->start + ONE_DAY - 1)
      file_size += DATE_LEN + 1;
    else
      file_size += DATE_LEN + 1 + DATE_LEN + 1;
  }
  ftruncate(fd, file_size);
  if (file_size == 0)
    return 0;
  char *buf = malloc(file_size + 1);
  if (buf == NULL)
    return -1;
  log_write(log, buf, file_size + 1);
  void *data = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    perror("map");
    free(buf);
    return -1;
  }
  memcpy(data, buf, file_size);
  free(buf);
  munmap(data, file_size);
  return file_size;
}

struct log_entry_s *log_get_last_entry(maki_uchi_log_t *log) {
  if (log->head.next == &log->head)
    return NULL;
  return container_of(log->head.next, struct log_entry_s, list);
}
