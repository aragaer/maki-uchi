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

void usage() {
  fprintf(stderr, "Usage: maki-uchi [-f filename] [number]\n");
  exit(EXIT_FAILURE);
}

void store_log(maki_uchi_log_t *log, char *file_name) {
  int fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1 && errno != ENOENT)
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

void (*print_log)(maki_uchi_log_t *log);

int parse_args(int argc, char *argv[], struct args *args) {
  int opt;
  while ((opt = getopt(argc, argv, "pf:o:")) != -1)
    switch (opt) {
    case 'f':
      args->file_name = optarg;
      break;
    case 'p':
      args->print_log = print_log_computer;
      break;
    case 'o': {
      int x, result;
      result = sscanf(optarg, "%d%n", &args->date_offset, &x);
      if (result != 1 || x < strlen(optarg)) {
        printf("%d %d\n", result, x);
        fprintf(stderr, "Failed to parse date offset: %s\nExpected integer.\n",
                optarg);
        return -1;
      }
    } break;
    default:
      usage();
      return 0;
    }
  return 0;
}

int main(int argc, char *argv[]) {
  maki_uchi_log_t log;
  log_init(&log);

  struct args args = {
    .date_offset = 0,
    .print_log = print_log_human,
    .file_name = "test.data",
  };
  int result = parse_args(argc, argv, &args);
  if (result != 0)
    return result;
  print_log = args.print_log;

  read_log(&log, args.file_name);
  if (optind < argc) {
    int count;
    if (sscanf(argv[optind], "%d", &count) != 1)
      usage();
    log_add(&log, count, time(NULL) + args.date_offset*ONE_DAY);
    store_log(&log, args.file_name);
  } else
    print_log(&log);
}
