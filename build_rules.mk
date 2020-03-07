##############################################
# Build strings (Object file prefix)
##############################################
#prefix is defined as arch_debug_target
#arch
PC_PREFIX = pc_
AVR_PREFIX = avr_
#debug
DBG_PREFIX = dbg_
FRE_PREFIX = fre_
#target
APP_PREFIX = app_
KERN_PREFIX = kern_


##############################################
#Compiler Parameters
##############################################
#c flags
ifeq ($(shell uname), Darwin)
PC_CFLAGS  = -g -Wall -Werror -I "$(PWD)" -fprofile-instr-generate -fcoverage-mapping
PC_CFLAGS_FAST = $(PC_CFLAGS) -O3
AVR_CFLAGS = -Wall -Werror -mmcu=atmega128 -O2
AVR_CFLAGS_FAST = $(AVR_CFLAGS) -O3
OS = "DARWIN"
endif

ifeq ($(shell uname), FreeBSD)
PC_CFLAGS  = -g -Wall -Werror
PC_CFLAGS_FAST = $(PC_CFLAGS) -O3
AVR_CFLAGS = -Wall -Werror -mmcu=atmega128
AVR_CFLAGS_FAST = $(AVR_CFLAGS) -O3
OS  = "BSD"
endif

ifeq ($(shell uname), Linux)
PC_CFLAGS  = -g -Wall -Werror -I "$(PWD)"
PC_CFLAGS_FAST = $(PC_CFLAGS) -O3
AVR_CFLAGS = -Wall -Werror -mmcu=atmega128 -gdwarf-2
AVR_CFLAGS_FAST = $(AVR_CFLAGS) -O3
OS = "LINUX"
endif
##############################################
# cc
PC_CC = gcc
AVR_CC = avr-gcc
##############################################
# Macros
##############################################
#Architecture
#ARCH_MACRO
ARCH_AVR = AVR_BUILD 
ARCH_PC = PC_BUILD
#############################################
#Debug
#DEBUG_MACRO
DEBUG_FRE = NO_DEBUG
DEBUG_DBG = DEBUG
#############################################
#Build
#BUILD_MACRO
BUILD_APP = APP_BUILD
BUILD_KERNEL = KERNEL_BUILD
MAKE_PROG = $_
#############################################
#combined strings
#############################################
#target for pc tests (non kernel build)
#string carries parameters to lower makefile. 
#string_name = TARGET CFLAGS CC ARCH_MACRO DEBUG_MACRO BUILD_MACRO
TEST_STRING = -e TARGET="$(PC_PREFIX)$(DBG_PREFIX)$(APP_PREFIX)" -e CFLAGS='$(PC_CFLAGS)' -e CC=$(PC_CC) -e ARCH_MACRO=$(ARCH_PC) -e DEBUG_MACRO=$(DEBUG_DBG) -e BUILD_MACRO=$(BUILD_APP) -e OS_MACRO=$(OS)
#target for pc kernel builds with debug enabled.
PC_STRING = -e TARGET="$(PC_PREFIX)$(DBG_PREFIX)$(KERN_PREFIX)" -e CFLAGS='$(PC_CFLAGS)' -e CC=$(PC_CC) -e ARCH_MACRO=$(ARCH_PC) -e DEBUG_MACRO=$(DEBUG_DBG) -e BUILD_MACRO=$(BUILD_KERNEL) -e OS_MACRO=$(OS)
#target for pc kernel without debug
PC_FRE_STRING = -e TARGET="$(PC_PREFIX)$(FRE_PREFIX)$(KERN_PREFIX)" -e CFLAGS='$(PC_CFLAGS_FAST)' -e CC=$(PC_CC) -e ARCH_MACRO=$(ARCH_PC) -e DEBUG_MACRO=$(DEBUG_FRE) -e BUILD_MACRO=$(BUILD_KERNEL) -e OS_MACRO=$(OS)
#target for avr kernel builds with debug enabled.
AVR_STRING = -e TARGET="$(AVR_PREFIX)$(DBG_PREFIX)$(KERN_PREFIX)" -e CFLAGS='$(AVR_CFLAGS)' -e CC=$(AVR_CC) -e ARCH_MACRO=$(ARCH_AVR) -e DEBUG_MACRO=$(DEBUG_DBG) -e BUILD_MACRO=$(BUILD_KERNEL) -e OS_MACRO=$(OS)
#target for avr kernel without debug
AVR_FRE_STRING = -e TARGET="$(AVR_PREFIX)$(FRE_PREFIX)$(KERN_PREFIX)" -e CFLAGS='$(AVR_CFLAGS_FAST)' -e CC=$(AVR_CC) -e ARCH_MACRO=$(ARCH_AVR) -e DEBUG_MACRO=$(DEBUG_FRE) -e BUILD_MACRO=$(BUILD_KERNEL) -e OS_MACRO=$(OS)

#############################################
#build rules
#############################################
%.o: 
	$(CC) $(CFLAGS) -D $(ARCH_MACRO) -D $(DEBUG_MACRO) -D $(BUILD_MACRO) -D $(OS_MACRO) -o $@ -c $<

%.out:
	$(CC) $(CFLAGS) -D $(ARCH_MACRO) -D $(DEBUG_MACRO) -D $(BUILD_MACRO) -D $(OS_MACRO) -o $@ $^

%.hex: %.out
	avr-objcopy -j .text -j .data -O ihex $< $@

%.elf: %.out
	cp $< $@
#########################################
#clean dir
.phony cleandir:
	rm -f $(DIR)/*.asm
	rm -f $(DIR)/*.out
	rm -f $(DIR)/*.o
	rm -f $(DIR)/core.*
	rm -f $(DIR)/*~
	rm -f $(DIR)/*.hex
	rm -f $(DIR)/*.elf
	rm -f $(DIR)/*.aws
	rm -f $(DIR)/*.core.
	rm -f $(DIR)/*.map
	rm -f $(DIR)/*.profraw
	rm -f $(DIR)/*.profdata
###########################
# tags 
ctags: 
	ctags --exclude=avr_hal.h --exclude=avr_hal.c --fields=+S -R .
#########################################
