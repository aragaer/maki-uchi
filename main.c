#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc, char *argv[] __attribute__((unused))) {
  if (argc == 2) {
    int fd = open("test.data", O_WRONLY | O_CREAT);
    close(fd);
  } else {
    struct stat buf;
    int result = stat("test.data", &buf);
    if (result == -1 && errno == ENOENT)
      printf("You did not do your maki-uchi today\n");
    else
      printf("You did your maki-uchi today\n");
  }
}
