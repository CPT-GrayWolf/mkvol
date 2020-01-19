# Default makefile for mkvol
CC = gcc
CFLAGS = 
TARGET = mkvol

RM = rm -r

SRCDIR = ./src
TARGETDIR = /usr/bin

all: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o $(TARGET)

install:
	@install -m 755 $(TARGET) $(TARGETDIR)/$(TARGET)
	@install -m 644 doc/$(TARGET).1 /usr/share/man/man1/$(TARGET).1
	@echo "Installing $(TARGET) in $(TARGETDIR)"

clean:
	@$(RM) $(TARGET)
