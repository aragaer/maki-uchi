CFLAGS := -Wall -Wextra -Werror

all: maki-uchi

test: maki-uchi-test maki-uchi
	./maki-uchi-test
	./test.sh

maki-uchi-test: CPPFLAGS += -DDEBUG
maki-uchi-test: test.o maki-uchi.o list.o

maki-uchi: main.c maki-uchi.o list.o

maki-uchi maki-uchi-test:
	$(CC) -o $@ $^

clean:
	rm -f maki-uchi maki-uchi-test *.o

.PHONY: all test clean
