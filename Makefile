TARGET := maxpixel
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

CC = gcc
CFLAGS = -c -Wall -g -Os -lpthread
LD = $(CC)
LDFLAGS =

.phony: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGET) $(OBJS)
