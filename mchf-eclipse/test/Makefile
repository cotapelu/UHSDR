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

CC          = gcc
CFLAGS      = -Wall -Wextra -std=c11 -O2 -I. -I mocks -I ../drivers/remote -I ../hal/include
LDFLAGS    = -lm
# Coverage (T68). Use -O0 so line counts map cleanly to source.
COVFLAGS   = -O0 -fprofile-arcs -ftest-coverage
COVREPORT  = coverage.txt
GCOVR      = $(shell command -v gcovr 2>/dev/null)
LCOV       = $(shell command -v lcov  2>/dev/null)

SRCS    = test.c \
          audio_driver_filters_standalone.c \
          test_audio_convolution_buffers.c \
          test_audio_translate_freq.c \
          test_cw_dial_offset.c \
          test_hal_uart.c \
          test_ui_config_getentry.c \
          test_ui_state_queries.c \
          test_psk_varicode.c \
          test_audio_management_balance.c \
          test_radio_power_calc.c \
          test_radio_sideband.c \
          test_rtty_baudot.c \
          test_cw_gen_helpers.c \
          test_cw_char_lookup.c \
          test_audio_nr_notch.c \
          test_ui_voltage_display.c \
          test_audio_alc_decay.c \
          test_audio_biquad_coeffs.c \
          test_cw_reverse_code.c \
          test_radio_freq_in_band.c \
          test_lcd_text_width.c \
          test_radio_freq_translation_mode.c \
          test_cw_decoder_spike.c \
          test_ui_menu_item_change.c \
          mocks/mock_hal_gpio.c \
          mocks/mock_hal_i2c.c \
          mocks/mock_hal_spi.c \
          mocks/mock_hal_uart.c

TARGET  = test_runner

.PHONY: all clean test coverage

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

test: $(TARGET)
	@echo "Running unit tests..."
	@./$(TARGET)

# T68: build with coverage instrumentation, run the suite, emit a text/summary report.
# Honest scope note: the suite mostly re-implements pure helpers inline, so this is
# coverage of the test shim + audio_driver_filters_standalone + mocks, NOT line
# coverage of drivers/. See docs/TODO.md Phase 67.2/67.4.
coverage:
	$(MAKE) clean
	$(CC) $(CFLAGS) $(COVFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)
	@echo "=== running instrumented suite ==="
	@./$(TARGET)
	@echo "" > $(COVREPORT)
	@echo "UHSDR host-suite coverage — $(shell date -u +%Y-%m-%dT%H:%M:%SZ)" >> $(COVREPORT)
ifdef GCOVR
	@gcovr -r .. --txt-summary >> $(COVREPORT) 2>/dev/null || true
else ifdef LCOV
	@$(LCOV) --capture --directory . --output-file $(COVREPORT).info --quiet 2>/dev/null || true
	@$(LCOV) --summary $(COVREPORT).info >> $(COVREPORT) 2>/dev/null || true
else
	@gcov -b -c $(SRCS:.c=.c) >/dev/null 2>&1 || true
endif
	@echo "Report written to $(COVREPORT)"

clean:
	rm -f $(TARGET) *.o *.gcda *.gcno coverage.txt coverage.txt.info *.gcov
