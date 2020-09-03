#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "entry.h"
#include "list.h"
#include "maki-uchi.h"
#include "reader.h"

#define ONE_DAY (24 * 60 * 60)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void insert_one_day_entry(maki_uchi_log_t *log, time_t start, int count) {
  time_t end = start + ONE_DAY - 1;
  insert_entry(log, create_entry(start, end, count));
}

static inline time_t round_to_day_start(time_t timestamp) {
  return timestamp - timestamp % ONE_DAY;
}

void log_add(maki_uchi_log_t *log, int count, time_t timestamp) {
  while (count > 0) {
    struct log_entry_s *entry = find_entry(log, timestamp);
    if (entry == NULL) {
      insert_one_day_entry(log, round_to_day_start(timestamp), MIN(count, DAILY_REQUIREMENT));
      count -= DAILY_REQUIREMENT;
      timestamp -= ONE_DAY;
    } else if (entry->count < DAILY_REQUIREMENT) {
      int missing = DAILY_REQUIREMENT - entry->count;
      int added = MIN(missing, count);
      if (DAYS(entry) > 1) {
        insert_one_day_entry(log, entry->end - ONE_DAY + 1, entry->count + added);
        entry->end -= ONE_DAY;
      } else
        entry->count += added;
      count -= added;
      timestamp -= ONE_DAY;
    } else
      timestamp = entry->start - 1;
  }
}

static size_t write_entry(struct log_entry_s *entry, char *buf, size_t bufsize) {
  struct tm tm;
  localtime_r(&entry->start, &tm);
  size_t result = strftime(buf, bufsize, "%Y.%m.%d", &tm);
  if (DAYS(entry) > 1) {
    time_t last_day = entry->end - ONE_DAY + 1;
    localtime_r(&last_day, &tm);
    result += strftime(buf+result, bufsize-result, "-%Y.%m.%d", &tm);
  }
  result += snprintf(buf+result, bufsize-result, " %d\n", entry->count);
  return result;
}

size_t log_write(maki_uchi_log_t *log, char *buf, size_t bufsize) {
  size_t result = 0;
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next)
    result += write_entry(from_list_head(item),
                          buf+result, bufsize-result);
  if (result < bufsize)
    buf[result] = '\0';
  return result;
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
  while (reader.state != END)
    reader_step(&reader);
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

#define DATE_LEN 10
#define SPACE_LEN 1
#define NEWLINE_LEN 1
#define DATE_SEP_LEN 1

static size_t number_len(int number) {
  int result = 0;
  while (number) {
    result++;
    number /= 10;
  }
  return result;
}

static size_t calculate_entry_size(struct log_entry_s *entry) {
  size_t result = DATE_LEN;
  if (DAYS(entry) > 1)
    result += DATE_SEP_LEN + DATE_LEN;
  result += SPACE_LEN + number_len(entry->count);
  result += NEWLINE_LEN;
  return result;
}

static size_t calculate_file_size(maki_uchi_log_t *log) {
  size_t result = 0;
  struct list_head *item;
  for (item = log->head.next; item != &log->head; item = item->next)
    result += calculate_entry_size(from_list_head(item));
  return result;
}

size_t log_write_file(maki_uchi_log_t *log, int fd) {
  size_t file_size = calculate_file_size(log);
  ftruncate(fd, file_size);
  if (file_size == 0)
    return 0;
  char *data = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    perror("map");
    return -1;
  }
  log_write(log, data, file_size);
  data[file_size - 1] = '\n';  // with this exact size last byte is equal to '\0'
  munmap(data, file_size);
  return file_size;
}
