#  -*-  mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-
####################################################################################
##                                                                                ##
##                                        UHSDR                                  ##
##               a powerful firmware for STM32 based SDR transceivers              ##
##                                                                                ##
##--------------------------------------------------------------------------------##
##                                                                                ##
##  File name:     Makefile                                                       ##
##  Description:   Host-based unit test build                                     ##
##  Last Modified:                                                                ##
##  Licence:		GNU GPLv3                                                      ##
####################################################################################

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -I.
LDFLAGS = -lm

SRCS    = test.c \
          test_audio_filters.c \
          audio_driver_filters_standalone.c

TARGET  = test_runner

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

test: $(TARGET)
	@echo "Running unit tests..."
	@./$(TARGET)

clean:
	rm -f $(TARGET) *.o
