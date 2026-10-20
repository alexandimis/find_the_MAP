CC      := gcc
LDFLAGS := -O0 -flto #-fno-omit-frame-pointer	
CFLAGS  := -Wall -g -fsanitize=address -lncursesw #-std=c11 	# 'lncursesw' is for wide character support
LDLIBS  := -lncurses #-lm
TARGET  := main

SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)
OBJS := $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

run: $(TARGET)
	./main
	make clean

fix:
	reset

clean:
	rm -f $(OBJS) $(TARGET)
