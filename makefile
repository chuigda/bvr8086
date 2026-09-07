#!/usr/bin/env bash

CFLAGS_BVR = -Iinclude -Isrc/include
HEADERS = $(wildcard include/*.h) $(wildcard src/include/*.h)
SRCS = Main.c $(wildcard src/**/*.c)
OBJS = $(SRCS:.c=.obj)
TARGET = bvr8086.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	@printf '\t%s\t%s\n' 'LINK' $@
	@$(CC) $(CFLAGS_BVR) $(CFLAGS) -o $@ $(OBJS)

%.obj: %.c $(HEADERS)
	@printf '\t%s\t%s\n' 'CC' $<
	@$(CC) $(CFLAGS_BVR) $(CFLAGS) -c -o $@ $<

run: dosbox_c/$(TARGET)
	@printf '\t%s\t%s\n' 'DOSBOX' $<
	@$(DOSBOX) -conf dosbox.conf

dosbox_c/$(TARGET): $(TARGET)
	@cp $< $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run
