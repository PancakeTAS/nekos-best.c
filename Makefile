SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
TARGET = nekosbest-c

MAKE = make
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g
LDFLAGS = -lcurl -lcjson

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ 

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb $(TARGET)

.PHONY: all clean run


