CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

INCLUDEDIR = include
SRCDIR = src
BUILDDIR = build

TARGET = balancer


SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:%.c=%.o)
TARGET = balancer

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))


all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^
	rm $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I $(INCLUDEDIR) -c -o $@ $^

clean:
	rm $(BUILDDIR)/$(TARGET)

.PHONY: all clean
