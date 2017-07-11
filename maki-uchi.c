#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  int fd = open("test.data", O_WRONLY, O_CREAT);
  close(fd);
}
