TEST_SOURCES := $(wildcard tests/*.c)
TEST_OBJECTS := $(TEST_SOURCES:.c=.o)

TEST_SOURCES_CPP := $(wildcard tests/*.cpp)
TEST_OBJECTS_CPP := $(TEST_SOURCES_CPP:.cpp=.o)

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g -Isrc
CPPFLAGS = -Wall -Wextra -Werror -pedantic -g -Isrc
LDFLAGS = -lcurl -lcjson

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $< -o $@

all: $(TEST_OBJECTS) $(TEST_OBJECTS_CPP)

test: $(TEST_OBJECTS) $(TEST_OBJECTS_CPP)
	@for obj in $(TEST_OBJECTS); do ./$$obj; done
	@for obj in $(TEST_OBJECTS_CPP); do ./$$obj; done

valgrind: $(TEST_OBJECTS) $(TEST_OBJECTS_CPP)
	@for obj in $(TEST_OBJECTS); do valgrind --leak-check=full ./$$obj; done
	@for obj in $(TEST_OBJECTS_CPP); do valgrind --leak-check=full ./$$obj; done

clean:
	rm -f $(TEST_OBJECTS) $(TEST_OBJECTS_CPP)

.PHONY: $(TEST_SOURCES) $(TEST_SOURCES_CPP) all test clean
