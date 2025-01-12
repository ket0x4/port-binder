# Makefile for building server and client applications

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g3 -Wall -Wextra -Werror

# Targets
TARGETS = server client

# Build rules
all: $(TARGETS)

%: %.c
	$(CC) -I. $< common.c -o $@ $(CFLAGS)

# Clean rule
clean:
	rm -f $(TARGETS)

.PHONY: all clean
