#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "computer.h"
#include "human.h"
#include "maki-uchi.h"
#include "print.h"

char *file_name = "test.data";

void usage() {
  fprintf(stderr, "Usage: maki-uchi [-f filename] [number]\n");
  exit(EXIT_FAILURE);
}

void store_log(maki_uchi_log_t *log) {
  int fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_write_file(log, fd);
  close(fd);
}

void read_log(maki_uchi_log_t *log) {
  int fd = open(file_name, O_RDONLY);
  if (fd == -1 && errno != ENOENT)
    perror("open");
  log_read_file(log, fd);
  close(fd);
}

void (*print_log)(maki_uchi_log_t *log) = print_log_human;

void parse_args(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "pf:")) != -1)
    switch (opt) {
    case 'f':
      file_name = optarg;
      break;
    case 'p':
      print_log = print_log_computer;
      break;
    default:
      usage();
      break;
    }
}

int main(int argc, char *argv[]) {
  maki_uchi_log_t log;
  log_init(&log);
  parse_args(argc, argv);
  read_log(&log);
  if (optind < argc) {
    int count;
    if (sscanf(argv[optind], "%d", &count) != 1)
      usage();
    log_add(&log, count, time(NULL));
    store_log(&log);
  } else
    print_log(&log);
}
