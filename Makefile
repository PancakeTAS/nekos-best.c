TEST_SOURCES := $(wildcard tests/*.c)
TEST_OBJECTS := $(TEST_SOURCES:.c=.o)

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g -Isrc
LDFLAGS = -lcurl -lcjson

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

all: $(TEST_OBJECTS)

test: $(TEST_OBJECTS)
	for obj in $(TEST_OBJECTS); do ./$$obj; done

clean:
	rm -f $(TEST_OBJECTS)

.PHONY: $(TEST_SOURCES) all test clean
