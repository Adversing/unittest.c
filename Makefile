CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = test_example
SRC_DIR = src
SOURCES = $(SRC_DIR)/unittest.c
OBJECTS = $(SOURCES:.c=.o)
INCLUDE_DIR = $(SRC_DIR)

.PHONY: all clean run install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) libunittest.a

libunittest.a: $(SRC_DIR)/unittest.o
	ar rcs $@ $^

install: libunittest.a $(SRC_DIR)/unittest.h
	mkdir -p /usr/local/lib /usr/local/include
	cp libunittest.a /usr/local/lib/
	cp $(SRC_DIR)/unittest.h /usr/local/include/