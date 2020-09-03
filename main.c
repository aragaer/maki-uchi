#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "computer.h"
#include "human.h"
#include "maki-uchi.h"
#include "print.h"

int usage() {
  fprintf(stderr, "Usage: maki-uchi [-f filename] [-o date_offset] [number]\n");
  exit(EXIT_FAILURE);
}

void store_log(maki_uchi_log_t *log, char *file_name) {
  int fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1)
    perror("open");
  log_write_file(log, fd);
  close(fd);
}

void read_log(maki_uchi_log_t *log, char *file_name) {
  int fd = open(file_name, O_RDONLY);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_read_file(log, fd);
  close(fd);
}

struct args {
  int date_offset;
  void (*print_log)(maki_uchi_log_t *);
  char *file_name;
};

int str_to_int_or_usage(char *str, const char *desc) {
  int result, bytes, value;
  result = sscanf(str, "%d%n", &value, &bytes);
  if (result == 1 && bytes == strlen(str))
    return value;
  fprintf(stderr, "Failed to parse %s: %s\nExpected integer.\n", desc, str);
  return usage();
}

void parse_args(int argc, char *argv[], struct args *args) {
  int opt;
  while ((opt = getopt(argc, argv, "pf:o:")) != -1)
    switch (opt) {
    case 'f':
      args->file_name = optarg;
      break;
    case 'p':
      args->print_log = print_log_computer;
      break;
    case 'o':
      args->date_offset = str_to_int_or_usage(optarg, "date offset");
      break;
    default:
      usage();
    }
}

int main(int argc, char *argv[]) {
  maki_uchi_log_t log;
  log_init(&log);

  struct args args = {
    .date_offset = 0,
    .print_log = print_log_human,
    .file_name = "test.data",
  };
  parse_args(argc, argv, &args);

  read_log(&log, args.file_name);
  if (optind < argc) {
    int count = str_to_int_or_usage(argv[optind], "number");
    log_add(&log, count, time(NULL) + args.date_offset * ONE_DAY);
    store_log(&log, args.file_name);
  } else
    args.print_log(&log);
}
