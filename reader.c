#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "reader.h"

#define DATE_LEN 10
#define ONE_DAY (24 * 60 * 60)

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

static void (*sm[])(struct read_runner *) = {
  [BEGIN] = &read_date,
  [ONE_DATE] = &read_hyphen,
  [SECOND_DATE] = &read_date,
  [COUNT] = &read_count,
  [EOL] = &read_eol,
};

void reader_step(struct read_runner *this) {
  sm[this->state](this);
}
