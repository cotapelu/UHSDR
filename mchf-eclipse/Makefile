#
# Author: Harald Baumgart DL4SAI
# everybody may  copy, use or modify this file
# there is no guarantee for anything by the author
#
# HB 20.8.2015
#
# Rev. 09/08/2015 corrected by Andreas Richter DF8OE 
# Rev. 2016-04-06 cleanup - Stephan HB9ocq
# Rev. 06/12/2016 possibility of choosing individual toolchain - Andreas Richter DF8OE
# Rev. 2017-01-06 HB9ocq - added versioning of build: extracted from source, propagated to env.vars
# Rev. 20/11/2018 RV9YW (m-chichikalov) - fixed limitation of parameter length on cmd.exe windows by passing all obj. as one file to linker

#  -*- makefile -*-
# if you want to build in a different directory than this one
# go to the directory, figure out the path from the new diretory you are in to this one and call make like this
# make -f replace_with_pfad_to_makefile/Makefile ROOTLOC=replace_with_pfad_to_makefile desired_maketarget
# for example to build the bootloader in the build-bl folder inside this directory
# make -f ../Makefile ROOTLOC=".." bootloader 

# to invoke asserts into the build use make with DEBUG=1; -> make -f "../Makefile" ROOTLOC=".." DEBUG=1 all

# you can set the MCU you want to build for here or via environment variable (F4 or F4-512KB or F7 or H7)
ifndef BUILDFOR
  BUILDFOR=F4
endif

# default value for building the mcHF UI board.
ifndef CONFIGFLAGS
  CONFIGFLAGS=-DUI_BRD_MCHF -DRF_BRD_MCHF
endif

# We have to use "override" since we pass some of these things from
# the commandline but then change / append to it
# see https://www.gnu.org/software/make/manual/html_node/Override-Directive.html#Override-Directive
# BUILDFOR, CONFIGFLAGS are affected by this


# F4-512KB results in "small build for F4"
ifeq ($(BUILDFOR),F4-512KB)
  FW_LINKERFILE_F4=arm-gcc-link_f4_flash512k.ld
  override BUILDFOR=F4
  override CONFIGFLAGS +=-DIS_SMALL_BUILD
else
  FW_LINKERFILE_F4=arm-gcc-link_f4_flash1024k.ld
endif

# DEBUG is numeric (0/1). Using ifdef here treated the default DEBUG=0 as
# enabled because the variable was defined. Keep release builds truly release.
ifeq ($(DEBUG),1)
  override CONFIGFLAGS += -DDEBUG -DUSE_FULL_ASSERT -DTRACE
else
  override CONFIGFLAGS += -DNDEBUG
endif

ifeq ($(BUILDFOR),F7)
  # NOTE: the transceiver name is not decided here. The board configuration
  # headers own it (hardware/board_configs/UHSDR_UI_*_config.h), see T80.
endif
ifeq ($(BUILDFOR),H7)
  # NOTE: the transceiver name is not decided here. The board configuration
  # headers own it (hardware/board_configs/UHSDR_UI_*_config.h), see T80.
endif

ROOTLOC=.
vpath %.c $(ROOTLOC)
vpath %.cpp $(ROOTLOC)
vpath %.S $(ROOTLOC)

# you can set these environment to your individual values here or via environment variables
# TRX_ID maximum lenght is 5 characters!!
#
# TRX_ID is the release artifact / DFU file base name ("fw-<TRX_ID>.bin" on the
# SD card) and therefore stays board independent. TRX_NAME is the human
# readable product name and is owned by the board configuration headers. Only
# an explicitly requested TRX_NAME is injected into the compilation (T80).
ifndef TRX_ID
  TRX_ID=mchf
endif

ifneq ($(strip $(TRX_NAME)),)
  # Single quotes keep a name containing spaces in one shell word. The double
  # quotes belong to the macro body, so the C code sees a string literal.
  override CONFIGFLAGS += -D'TRX_NAME="$(TRX_NAME)"'
endif

# If you want to hold different toolchains on Linux in /opt you can get them from
# https://launchpad.net/gcc-arm-embedded . Copy unpacked files as 'root' to /opt .
# If you want to use other toolchain than system-wide  installed proceed as following:
# Type on a terminal
# OPT_GCC_ARM=/opt/folder-of-your-toolchain
# export OPT_GCC_ARM
# Now 'make all' uses choosen toolchain instead of system wide installed.
# If yu want to switch back to system wide type
# OPT_GCC_ARM=

ifdef OPT_GCC_ARM
  PREFIX = $(OPT_GCC_ARM)/bin/
else
  PREFIX = 
endif

# Under MacOS we have to use gsed instead of sed
# This mechanism can be used also for other flavours
OS := $(shell uname)
SED = sed
ifeq ($(OS),Darwin)
  SED = gsed
endif

COMPILEFLAGS := -D_GNU_SOURCE -D'TRX_ID="$(TRX_ID)"' $(CONFIGFLAGS) -DUSE_HAL_DRIVER\
	-DFDV_ARM_MATH -DFREEDV_MODE_EN_DEFAULT=0 -DFREEDV_MODE_1600_EN=1 -DCODEC2_MODE_EN_DEFAULT=0 -DCODEC2_MODE_1300_EN=1\
	-ffunction-sections -fdata-sections -flto -Wall -Wuninitialized -Wextra -Wimplicit-function-declaration -Wno-unused-parameter -Wmissing-prototypes -Wno-unused-function -Wno-sign-compare -g3 -fstack-usage

# identifying of "official builds by DF8OE"
ifneq (,$(wildcard ../DF8OE))
  COMPILEFLAGS += -DOFFICIAL_BUILD=\"1\"
endif

# compilation options
MACHFLAGS_F4 := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -DARM_MATH_CM4 -DCORTEX_M4 -DSTM32F407xx -D__FPU_PRESENT=1U
MACHFLAGS_F7 := -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16    -mthumb -DARM_MATH_CM7 -DCORTEX_M7 -DSTM32F767xx -D__FPU_PRESENT=1
MACHFLAGS_H7 := -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16    -mthumb -DARM_MATH_CM7 -DCORTEX_M7 -DSTM32H743xx -D__FPU_PRESENT=1

# The STM32 UART HAL driver source (stm32f*xx_hal_uart.c) is entirely wrapped in
# "#ifdef HAL_UART_MODULE_ENABLED" and every basesw stm32f*xx_hal_conf.h ships
# that macro commented out, so the driver compiles to an EMPTY object. hal_uart
# therefore had no implementation to link against. basesw is vendor-owned and
# must not be edited (AGENTS.md 5.2), so the module is enabled from here instead.
HALUARTFLAGS := -DHAL_UART_MODULE_ENABLED
MACHFLAGS_F4 += $(HALUARTFLAGS)
MACHFLAGS_F7 += $(HALUARTFLAGS)
MACHFLAGS_H7 += $(HALUARTFLAGS)

BASECFLAGS_F4  = $(MACHFLAGS_F4)  $(COMPILEFLAGS) $(EXTRACFLAGS)
BASECFLAGS_F7  = $(MACHFLAGS_F7)  $(COMPILEFLAGS) $(EXTRACFLAGS) -DFREEDV_MODE_700D_EN=1 -DCODEC2_MODE_700C_EN=1
BASECFLAGS_H7  = $(MACHFLAGS_H7)  $(COMPILEFLAGS) $(EXTRACFLAGS) -DFREEDV_MODE_700D_EN=1 -DCODEC2_MODE_700C_EN=1

LINKERLOC=$(ROOTLOC)/linker

LDFLAGS_BASE = -L$(LINKERLOC) -flto -g3 $(CFLAGS) $(CXXFLAGS)
LDFLAGS_F4 = $(LDFLAGS_BASE) $(MACHFLAGS_F4)
LDFLAGS_F7 = $(LDFLAGS_BASE) $(MACHFLAGS_F7) 
LDFLAGS_H7 = $(LDFLAGS_BASE) $(MACHFLAGS_H7) 

LIBS :=--specs=nosys.specs --specs=nano.specs

# ------------- nothing to change below this line ---------------------- 

# propagate version info from source to environment variables
PROJECT_VERSION_FILE := $(ROOTLOC)/src/uhsdr_version.h

ifdef MSYSTEM
$(eval  $(shell $(SED) -n -e \'s/\"//g\' -e '/[^-]UHSDR_VER_MAJOR/{s!\#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e \'s/\"//g\' -e '/[^.]UHSDR_VER_MINOR/{s!\#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e \'s/\"//g\' -e '/[^.]UHSDR_VER_RELEASE/{s!\#define\s*!!; s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
else
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/UHSDR_VER_MAJOR/{s!#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/UHSDR_VER_MINOR/{s!#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/UHSDR_VER_RELEASE/{s!#define\s*!!; s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
endif

UHSDR_VER_TAINT := $(shell  git status . | grep --quiet 'working directory clean' && echo "" || echo "+")

BOOTLOADER=bl-$(TRX_ID)
FIRMWARE=fw-$(TRX_ID)

# Every subdirectory with header files must be mentioned here
include $(ROOTLOC)/include.mak

# every source-file has to be mentioned here 
include $(ROOTLOC)/files.mak

include $(ROOTLOC)/bootloader.mak

ifeq ($(BUILDFOR),F4)

$(BOOTLOADER).elf : CFLAGS = ${BASECFLAGS_F4} -Os -DBOOTLOADER_BUILD
$(FIRMWARE).elf : CFLAGS = ${BASECFLAGS_F4} -O2
$(BOOTLOADER).elf : LDFLAGS = ${LDFLAGS_F4} -T$(LINKERLOC)/arm-gcc-link-bootloader_f4.ld
$(FIRMWARE).elf : LDFLAGS = ${LDFLAGS_F4} -T$(LINKERLOC)/$(FW_LINKERFILE_F4)

	# Every subdirectory with header files must be mentioned here
	include $(ROOTLOC)/f4-include.mak

	# every source-file has to be mentioned here 
	include $(ROOTLOC)/f4-files.mak

	include $(ROOTLOC)/f4-bootloader.mak

endif
ifeq ($(BUILDFOR),F7)

$(BOOTLOADER).elf: CFLAGS = ${BASECFLAGS_F7} -Os -DBOOTLOADER_BUILD
$(FIRMWARE).elf : CFLAGS = ${BASECFLAGS_F7} -O2
$(BOOTLOADER).elf : LDFLAGS = ${LDFLAGS_F7} -T$(LINKERLOC)/arm-gcc-link-bootloader_f7.ld
$(FIRMWARE).elf : LDFLAGS = ${LDFLAGS_F7} -T$(LINKERLOC)/arm-gcc-link_f7.ld

	# Every subdirectory with header files must be mentioned here
	include $(ROOTLOC)/f7-include.mak

	# every source-file has to be mentioned here 
	include $(ROOTLOC)/f7-files.mak

	include $(ROOTLOC)/f7-bootloader.mak

endif
ifeq ($(BUILDFOR),H7)

$(BOOTLOADER).elf: CFLAGS = ${BASECFLAGS_H7} -Os -DBOOTLOADER_BUILD
$(FIRMWARE).elf : CFLAGS = ${BASECFLAGS_H7} -O2
$(BOOTLOADER).elf : LDFLAGS = ${LDFLAGS_H7} -T$(LINKERLOC)/arm-gcc-link-bootloader_h7.ld
$(FIRMWARE).elf : LDFLAGS = ${LDFLAGS_H7} -T$(LINKERLOC)/arm-gcc-link_h7.ld

	# Every subdirectory with header files must be mentioned here
	include $(ROOTLOC)/h7-include.mak

	# every source-file has to be mentioned here 
	include $(ROOTLOC)/h7-files.mak

	include $(ROOTLOC)/h7-bootloader.mak

endif

INC_DIRS = $(foreach d, $(SUBDIRS) $(HAL_SUBDIRS), -I$(ROOTLOC)/$d)

ifdef VERBOSE
	VPRE:=
else
	VPRE:=@
endif

CC      = $(VPRE)$(PREFIX)arm-none-eabi-gcc
CXX     = $(VPRE)$(PREFIX)arm-none-eabi-g++
OC      = $(VPRE)$(PREFIX)arm-none-eabi-objcopy
SZ      = $(VPRE)$(PREFIX)arm-none-eabi-size
HEX2DFU = $(VPRE)python3 $(ROOTLOC)/support/hex2dfu/hex2dfu.py

# ---------------------------------------------------------------------------
# Configuration fingerprint / dirty-tree build safety (Phase 68, T75)
# ---------------------------------------------------------------------------
# Objects are emitted in-tree for historical reasons, so their names do not
# encode MCU/compiler flags. Without an explicit fingerprint, switching from
# F4 to H7 can leave the old ELF in place and report success. Hash every input
# that can change generated code, then invalidate all current objects when the
# fingerprint changes. An unchanged fingerprint does not touch the stamp, so
# normal incremental builds remain incremental.
BUILD_CONFIG_STAMP := .uhsdr_build_config

ifeq ($(BUILDFOR),F4)
  BUILD_CONFIG_CFLAGS := $(BASECFLAGS_F4)
  BUILD_CONFIG_LDFLAGS := $(LDFLAGS_F4)
  BUILD_CONFIG_LINKER := $(FW_LINKERFILE_F4)
else ifeq ($(BUILDFOR),F7)
  BUILD_CONFIG_CFLAGS := $(BASECFLAGS_F7)
  BUILD_CONFIG_LDFLAGS := $(LDFLAGS_F7)
  BUILD_CONFIG_LINKER := $(LINKERLOC)/arm-gcc-link_f7.ld
else ifeq ($(BUILDFOR),H7)
  BUILD_CONFIG_CFLAGS := $(BASECFLAGS_H7)
  BUILD_CONFIG_LDFLAGS := $(LDFLAGS_H7)
  BUILD_CONFIG_LINKER := $(LINKERLOC)/arm-gcc-link_h7.ld
endif

BUILD_CONFIG_TOOLCHAIN := $(shell $(PREFIX)arm-none-eabi-gcc --version 2>/dev/null | head -1)
BUILD_CONFIG_FINGERPRINT := $(shell { \
  printf '%s\n' \
    'BUILDFOR=$(BUILDFOR)' \
    'BOARD=$(BOARD)' \
    'DEBUG=$(DEBUG)' \
    'TRX_ID=$(TRX_ID)' \
    'TRX_NAME=$(TRX_NAME)' \
    'CONFIGFLAGS=$(CONFIGFLAGS)' \
    'CFLAGS=$(BUILD_CONFIG_CFLAGS)' \
    'LDFLAGS=$(BUILD_CONFIG_LDFLAGS)' \
    'LINKER=$(BUILD_CONFIG_LINKER)' \
    'TOOLCHAIN=$(BUILD_CONFIG_TOOLCHAIN)'; \
} | sha256sum | cut -d ' ' -f 1)

POSTCOMPILE = @mv -f $*.Td $*.d && touch $@
DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td


ECHO = @echo

ifdef SystemRoot  # WINxx
    RM = del /Q
    FixPath = $(subst /,\,$1)
else
    RM = rm -f
    FixPath = $1
endif

# how to compile individual object files
ALL_SRC:=$(SRC) $(DSPLIB_SRC) $(HAL_SRC) $(BL_SRC) $(BL_HAL_SRC)

OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRC:.cpp=.o)))
DSPLIB_OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(DSPLIB_SRC:.cpp=.o)))
HAL_OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(HAL_SRC:.cpp=.o)))
# Bootloader objects live in a separate tree. Several bootloader sources have
# the same paths as firmware sources (for example ui_lcd_hy28.c); using the
# same .o path made a firmware object silently satisfy a bootloader link.
BL_OBJ_DIR := build/bootloader
BL_OBJS := $(addprefix $(BL_OBJ_DIR)/,$(patsubst %.S,%.o,$(patsubst %.c,%.o,$(BL_SRC:.cpp=.o))))
BL_HAL_OBJS := $(addprefix $(BL_OBJ_DIR)/,$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(BL_HAL_SRC:.c=.o))))

ALL_OBJS := $(OBJS) $(DSPLIB_OBJS) $(HAL_OBJS) $(BL_OBJS) $(BL_HAL_OBJS)

# ---------------------------------------------------------------------------
# Identity introspection (Phase 68, T80)
# ---------------------------------------------------------------------------
# Print the identity macros (TRX_NAME/TRX_ID) exactly as the preprocessor
# resolves them for the current configuration. scripts/check_identity.sh uses
# this to guard the per board product identity without running a full build.
# The probe is never linked into a firmware or bootloader.
IDENTITY_PROBE ?= $(ROOTLOC)/../scripts/identity_probe.c

.PHONY: print-identity
print-identity:
	@$(CC) $(BUILD_CONFIG_CFLAGS) -std=gnu11 $(INC_DIRS) -E -dM $(IDENTITY_PROBE) \
		| grep -E '^#define (TRX_NAME|TRX_ID) '


$(DSPLIB_OBJS): EXTRA_CFLAGS:= -Wno-strict-aliasing
basesw/%/usbh_mtp.o: CFLAGS += -Wno-implicit-fallthrough
basesw/%/usbh_msc.o: CFLAGS += -Wno-implicit-fallthrough
drivers/usb/%/usbh_hid.o: CFLAGS += -Wno-implicit-fallthrough
basesw/%stm32h7xx_hal_spi.o: CFLAGS += -Wno-strict-aliasing
basesw/%stm32f4xx_ll_usb.o: CFLAGS += -Wno-attributes
basesw/%stm32f7xx_ll_usb.o: CFLAGS += -Wno-attributes
basesw/%stm32h7xx_ll_usb.o: CFLAGS += -Wno-attributes

.S.o:
	$(ECHO) "  [CC] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEPFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.c.o:
	$(ECHO) "  [CC] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEPFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.cxx.o:
	$(ECHO) "  [CXX] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.cpp.o:
	$(ECHO) "  [CXX] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

# Bootloader-only object tree. The source stem remains relative to ROOTLOC,
# so vpath can find it, while $@ keeps firmware and bootloader objects apart.
$(BL_OBJ_DIR)/%.o: %.c
	$(ECHO) "  [CC-BL] $@"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEPFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

$(BL_OBJ_DIR)/%.o: %.S
	$(ECHO) "  [AS-BL] $@"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEPFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.PHONY: FORCE
FORCE:

$(BUILD_CONFIG_STAMP): FORCE
	@current=""; \
	if [ -f "$@" ]; then current="$$(cat "$@")"; fi; \
	if [ "$$current" != "$(BUILD_CONFIG_FINGERPRINT)" ]; then \
	  echo "[CFG] configuration changed; invalidating stale objects"; \
	  rm -f $(ALL_OBJS) $(FIRMWARE).elf $(FIRMWARE).bin $(FIRMWARE).hex $(FIRMWARE).dfu $(FIRMWARE).map \
	        $(BOOTLOADER).elf $(BOOTLOADER).bin $(BOOTLOADER).hex $(BOOTLOADER).dfu $(BOOTLOADER).map \
	        *.ltrans*; \
	  printf '%s\n' "$(BUILD_CONFIG_FINGERPRINT)" > "$@.tmp"; \
	  mv -f "$@.tmp" "$@"; \
	fi

$(ALL_OBJS): $(BUILD_CONFIG_STAMP)

%.bin: %.elf
	$(ECHO) "  [OBJC] $@"
	$(SZ) $<
	$(OC) -v -O binary $< $@

%.hex: %.elf
	$(ECHO) "  [BIN] $@"
	$(SZ) $<
	$(OC) -v -O ihex $< $@

%.dfu: %.hex
	$(ECHO) "  [H2D] $@"
	$(SZ) $<
	$(HEX2DFU) $< $@

%.d: ;

.PRECIOUS: %.d

include $(wildcard $(patsubst %,%.d,$(basename $(ALL_SRC))))

# ---------------------------------------------------------
#  BUILT-IN HELP
#

define THISMAKEFILENAME
$(word 2,$Workfile: Makefile $ )
endef

# default (first) make goal
.PHONY: help
help:  
	# shows all make goals of this file (the text you are reading)
	@grep --after-context=1 --extended-regexp '^[[:alnum:]_-]+:[[:blank:]]{2,}' $(THISMAKEFILENAME)

# ---------------------------------------------------------

.PHONY: all clean docs docs-clean help


all:  firmware $(TRX_ID).handbook
	# compile the firmware ARM-executables .bin / .elf and .dfu, generate .map 

firmware:  $(FIRMWARE) 
	# compile the firmware ARM-executables .bin / .elf and .dfu, generate .map 
	@echo "using \c"
	$(CC) --version | grep gcc

bootloader:  $(BOOTLOADER)
	# compile the bootloader ARM-executables .bin / .elf and .dfu, generate .map 
	@echo "using \c"
	$(CC) --version | grep gcc

$(FIRMWARE): $(FIRMWARE).elf $(FIRMWARE).dfu $(FIRMWARE).bin

$(BOOTLOADER):  $(BOOTLOADER).bin $(BOOTLOADER).dfu
	# compile the bootloader ARM-executables .bin / .elf and .dfu for mcHF SDR TRx, generate .map and .dmp

# compilation
$(FIRMWARE).elf:  $(HAL_OBJS) $(DSPLIB_OBJS) $(OBJS)
	$(ECHO) "  [LD] $@"
	@$(file > ./firmware_obj_list.lst,$^)
	$(CXX) $(LDFLAGS) -Xlinker --gc-sections -Llibs -Wl,-Map,$(FIRMWARE).map -o$@ @./firmware_obj_list.lst $(LIBS)
	$(RM) $(call FixPath,firmware_obj_list.lst)

# compilation
$(BOOTLOADER).elf:  $(BL_HAL_OBJS) $(BL_OBJS)
	$(ECHO) "  [LD] $@"
	@$(file > ./bootloader_obj_list.lst,$^)
	$(CXX) $(LDFLAGS) -Xlinker --gc-sections -Llibs -Wl,-Map,$(BOOTLOADER).map -o$@ @./bootloader_obj_list.lst $(LIBS)
	$(RM) $(call FixPath,bootloader_obj_list.lst)

$(TRX_ID).handbook:
	@$(ROOTLOC)/support/ui/menu/mk-menu-handbook auto

$(TRX_ID).version:
	# the build artifacts SHOULD identify as
	@printf "Version %s.%s.%s%s\n" $(UHSDR_VER_MAJOR) $(UHSDR_VER_MINOR) $(UHSDR_VER_RELEASE) $(UHSDR_VER_TAINT)

# cleaning rule
clean-firmware:  
	# remove the firmware executables, map, dmp and all object files (.o)
	$(RM) $(call FixPath,$(OBJS))	
	$(RM) $(call FixPath,$(HAL_OBJS))
	$(RM) $(call FixPath,$(DSPLIB_OBJS))
	$(RM) $(call FixPath,$(FIRMWARE).elf)
	$(RM) $(call FixPath,$(FIRMWARE).dfu)
	$(RM) $(call FixPath,$(FIRMWARE).bin)
	$(RM) $(call FixPath,$(FIRMWARE).map)

clean-bootloader:  
	# remove the bootloader executables, map, dmp and all object files (.o) (except libs object files)
	$(RM) $(call FixPath,$(BL_OBJS))
	$(RM) $(call FixPath,$(BL_HAL_OBJS))
	$(RM) $(call FixPath,$(BOOTLOADER).elf)
	$(RM) $(call FixPath,$(BOOTLOADER).dfu)
	$(RM) $(call FixPath,$(BOOTLOADER).bin)
	$(RM) $(call FixPath,$(BOOTLOADER).map)

clean-libs:  
	# remove libs object files (.o)
	$(RM) $(call FixPath,$(DSPLIB_OBJS))
	$(RM) $(call FixPath,$(HAL_OBJS))

clean:  clean-firmware clean-bootloader clean-libs
	# remove the executables, map, dmp and all generated object/dependency files
	$(RM) $(call FixPath,*~)
	$(RM) -r $(call FixPath,$(BL_OBJ_DIR))
	$(RM) $(call FixPath,$(BUILD_CONFIG_STAMP))
	@find $(ROOTLOC) -type f \( \
	  -name '*.o' -o -name '*.d' -o -name '*.Td' -o \
	  -name '*.su' -o -name '*.ltrans*' \
	\) -delete

docs:  
	# generate source docs as per "Doxyfile"
	doxygen Doxyfile

docs-clean:  
	# remove docs
	# as defined in file "Doxyfile" OUTPUT_DIRECTORY
	$(RM) --recursive --verbose $(call FixPath,$(ROOTLOC)/../docs)

gcc-version:  
	# the build will be done using
	$(CC) --version | grep gcc

handbook-test:  
	# extract UI Menu Descriptor data from source code and generate graph + table for handbook in different directory for test purposes
	@$(ROOTLOC)/support/ui/menu/mk-menu-handbook test

handbook-ui-menu:  
	# extract UI Menu Descriptor data from source code and generate graph + table for handbook
	@$(ROOTLOC)/support/ui/menu/mk-menu-handbook

handbook-ui-menu-clean:  
	# remove generated UI Menu files
	$(RM) $(ROOTLOC)/support/ui/menu/uhsdr-logo.png
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure.py*
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_graph.gv
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_graph.svg
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_graph.png
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_mdtable.md
	$(RM) $(ROOTLOC)/support/ui/menu/menu-handbook-build.timestamp

handy:  
	# rm all .o (but not executables, .map and .dmp)
	$(RM) $(call FixPath,$(ALL_OBJS))
	$(RM) $(call FixPath,*~)

release:  
	# generate quick operating guide
	/@inkscape --export-png=$(ROOTLOC)/useful_manuals/mcHF-quick-manual.png $(ROOTLOC)/useful_manuals/mcHF-quick-manual.svg
	@inkscape --export-pdf=$(ROOTLOC)/useful_manuals/mcHF-quick-manual.pdf $(ROOTLOC)/useful_manuals/mcHF-quick-manual.svg

stack-report: 
	# prints top-10 deepest call-stacks per target from LTO stack-usage .su files
	@python3 $(ROOTLOC)/support/stack-report.py $(ROOTLOC)

# EOFILE
