CFLAGS := -Wall -Wextra -Werror

COMMON_O = maki-uchi.o list.o log.o entry.o reader.o
PRINT_O = print.o computer.o human.o
TEST_O = test/test.o test/test_read.o test/test_file.o

all: maki-uchi

test: maki-uchi-test maki-uchi
	./maki-uchi-test
	./test.sh

maki-uchi-test: CPPFLAGS += -DDEBUG -I.
maki-uchi-test: $(TEST_O) $(COMMON_O)

maki-uchi: main.c $(COMMON_O) $(PRINT_O)

maki-uchi maki-uchi-test:
	$(CC) -o $@ $^

clean:
	rm -f maki-uchi maki-uchi-test *.o

.PHONY: all test clean
