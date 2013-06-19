#include terminal.mk

CC     := g++
LD     := ld
OUTDIR := out

BUILD_TYPE := debug

#CFLAGS=-std=c99 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -Iinclude -Iinclude/metodo -m32
override CFLAGS += -DENABLE_DEBUGGER_SUPPORT -DENABLE_DISASSEMBLER -DV8_ENABLE_CHECKS -DOBJECT_PRINT -I./src  -Wall -Werror -W -Wno-unused-parameter -Wnon-virtual-dtor -pthread -fno-rtti -fno-exceptions -pedantic -ansi -fvisibility=hidden -m32 -g -O0 -Wall -Werror -W -Wno-unused-parameter -Wnon-virtual-dtor -Woverloaded-virtual -MMD

override LDFLAGS += -nostdlib -g -melf_i386

override ASFLAGS += -felf32

# Special build flags. Use them like this: "make library=shared"

ifeq ($(library), shared)
  CFLAGS += -Dcomponent=shared_library
endif
ifdef component
  CFLAGS += -Dcomponent=$(component)
endif
# console=readline
ifdef console
  CFLAGS += -Dconsole=$(console)
endif
# disassembler=on
ifeq ($(disassembler), on)
  CFLAGS += -Dv8_enable_disassembler=1
endif
# objectprint=on
ifeq ($(objectprint), on)
  CFLAGS += -Dv8_object_print=1
endif
# snapshot=off
ifeq ($(snapshot), off)
  CFLAGS += -Dv8_use_snapshot='false'
endif
# gdbjit=on
ifeq ($(gdbjit), on)
  CFLAGS += -Dv8_enable_gdbjit=1
endif
# liveobjectlist=on
ifeq ($(liveobjectlist), on)
  CFLAGS += -Dv8_use_liveobjectlist=true
endif
# vfp3=off
ifeq ($(vfp3), off)
  CFLAGS += -Dv8_can_use_vfp_instructions=false
else
  CFLAGS += -Dv8_can_use_vfp_instructions=true
endif
# debuggersupport=off
ifeq ($(debuggersupport), off)
  CFLAGS += -Dv8_enable_debugger_support=0
endif
# soname_version=1.2.3
ifdef soname_version
  CFLAGS += -Dsoname_version=$(soname_version)
endif
# werror=no
ifeq ($(werror), no)
  CFLAGS += -Dwerror=''
endif
# presubmit=no
ifeq ($(presubmit), no)
  TESTFLAGS += --no-presubmit
endif
# strictaliasing=off (workaround for GCC-4.5)
ifeq ($(strictaliasing), off)
  CFLAGS += -Dv8_no_strict_aliasing=1
endif
# regexp=interpreted
ifeq ($(regexp), interpreted)
  CFLAGS += -Dv8_interpreted_regexp=1
endif

ifeq ($(target), ia32)
	CFLAGS += -DV8_TARGET_ARCH_IA32
endif

ifeq ($(snapshot), on)
	# snapshot=on
	CFLAGS += -DV8_USE_SNAPSHOT='true'
else
	# snapshot=off
	CFLAGS += -DV8_USE_SNAPSHOT='false'
endif

# debuggersupport=off
ifeq ($(debuggersupport), off)
	CFLAGS += -DENABLE_DEBUGGER_SUPPORT=0
endif

ifdef debug
	CFLAGS += -DDEBUG
endif

# Are we *SERIOUS*?
# Here's what we do:
#   1. get list all .cc files in src
#   2. disregard src/platform-*.cc, as they are...platform-specific
#   3. disregard *dll* because v8dll-main.cc and v8preparserdll-main.cc are 
#      Windows-specific
#   4. We add back platform-specific crud later on.
SRCFILES := $(shell ls src/*.cc | grep -vE '^src/platform-' | grep -v 'dll')
OBJFILES := $(patsubst %.cc, %.o, $(SRCFILES))


all: ${OBJFILES}

#-include $(find ./src -name '*.d')
%.o: %.cc
#	@$(call STATUS,"COMPILE ",$^)
	${CC} ${CFLAGS} -MP -MT "$*.d $*.o"  -c $< -o $@

#todo:
#	@./tools/todo.sh

sloc:
	@sloccount ./src ./include | grep "(SLOC)"

clean:
	@find ./src -name '*.o'   -delete
	@find ./src -name '*.lib' -delete
	@find ./src -name '*.exe' -delete
	@find ./src -name '*.d'   -delete
	@rm -rf out/*.release out/*.debug

.PHONY: all clean test qemu qemu-monitor bochs todo sloc
