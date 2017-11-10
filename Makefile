CFLAGS := -Wall -Wextra -Werror

COMMON_O = maki-uchi.o list.o log.o entry.o

all: maki-uchi

test: maki-uchi-test maki-uchi
	./maki-uchi-test
	./test.sh

maki-uchi-test: CPPFLAGS += -DDEBUG
maki-uchi-test: test.o $(COMMON_O)

maki-uchi: main.c $(COMMON_O)

maki-uchi maki-uchi-test:
	$(CC) -o $@ $^

clean:
	rm -f maki-uchi maki-uchi-test *.o

.PHONY: all test clean
