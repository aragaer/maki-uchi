#define _XOPEN_SOURCE 500
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "maki-uchi.h"
#include "list.h"

#define ONE_DAY (24 * 60 * 60)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define DAYS(e) (((e)->end - (e)->start)/ONE_DAY)

void log_add(maki_uchi_log_t *log, int count, time_t timestamp) {
  while (count > 0) {
    struct log_entry_s *entry = find_entry(log, timestamp);
    if (entry == NULL) {
      time_t start = timestamp - timestamp % ONE_DAY;
      time_t end = start + ONE_DAY - 1;
      insert_entry(log, create_entry(start, end, MIN(count, DAILY_REQUIREMENT)));
      count -= DAILY_REQUIREMENT;
      timestamp -= ONE_DAY;
    } else if (entry->count < DAILY_REQUIREMENT) {
      int missing = DAILY_REQUIREMENT - entry->count;
      int added = MIN(missing, count);
      if (entry->end - entry->start > ONE_DAY) {
	insert_entry(log, create_entry(entry->end - ONE_DAY + 1,
				       entry->end, entry->count + added));
	entry->end -= ONE_DAY;
      } else
	entry->count += added;
      count -= added;
      timestamp -= ONE_DAY;
    } else
      timestamp = entry->start - 1;
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
    result += snprintf(buf+result, bufsize-result, " %d\n", entry->count);
    if (result < bufsize) {
      buf[result] = '\0';
    }
  }
  return result;
}

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

#define DATE_LEN 10

static void read_date(struct read_runner *this) {
  if (this->data_left < DATE_LEN
      || strptime(this->ptr, "%Y.%m.%d", this->tm) == NULL) {
    this->state = END;
  } else {
    this->ptr += DATE_LEN;
    this->data_left -= DATE_LEN;
    if (this->state == BEGIN) {
      this->entry = alloc_entry();
      this->entry->start = mktime(this->tm);
      this->state = ONE_DATE;
    } else
      this->state = COUNT;
  }
}

static void read_hyphen(struct read_runner *this) {
  if (this->data_left == 0 || *this->ptr != '-')
    this->state = COUNT;
  else {
    this->ptr++;
    this->data_left--;
    this->state = SECOND_DATE;
  }
}

static void read_eol(struct read_runner *this) {
  this->entry->end = mktime(this->tm) + ONE_DAY - 1;
  insert_entry(this->log, this->entry);
  this->entry = NULL;
  if (this->data_left <= 0 || *this->ptr != '\n')
    this->state = END;
  else {
    this->ptr++;
    this->data_left--;
    this->state = BEGIN;
  }
}

static void read_count(struct read_runner *this) {
  if (this->data_left < 1 || *this->ptr != ' ') {
    // old format - no count
    this->state = EOL;
    this->entry->count = DAILY_REQUIREMENT;
  } else {
    char count_buf[this->data_left + 1];
    memcpy(count_buf, this->ptr, this->data_left);
    count_buf[this->data_left] = 0;
    int bytes;
    if (sscanf(count_buf, " %d%n", &this->entry->count, &bytes) != 1)
      this->state = END;
    else {
      this->state = EOL;
      this->ptr += bytes;
      this->data_left -= bytes;
    }
  }
}

void (*sm[])(struct read_runner *) = {
  [BEGIN] = read_date,
  [ONE_DATE] = read_hyphen,
  [SECOND_DATE] = read_date,
  [COUNT] = read_count,
  [EOL] = read_eol,
};

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
  while (reader.state != END)
    sm[reader.state](&reader);
  free(reader.entry);
  return reader.data_left ? -1 : 0;
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
    int c = entry->count;
    file_size++;
    while (c) {
      file_size++;
      c /= 10;
    }
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
