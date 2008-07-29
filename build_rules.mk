##############################################
# Build strings (Prefix)
##############################################
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
#C flags
PC_CFLAGS = -p -g -Wall 
ifeq ($(shell uname), Darwin)
AVR_CFLAGS = -Wall -mmcu=atmega128 -O2
else
AVR_CFLAGS = -Wall -mmcu=atmega128 -gdwarf-2
endif
##############################################
# cc
PC_CC = gcc
AVR_CC = avr-gcc
##############################################
# macros
##############################################
#Archetecture
ARCH_AVR = AVR
ARCH_PC = PC
#############################################
#Debug
DEBUG_FRE = FRE
DEBUG_DBG = DBG
#############################################
#Build
BUILD_APP = APP
BUILD_KERNEL = KERNEL
#############################################
#combined strings
#############################################
#target for pc tests (non kernel build)
TEST_STRING = -e TARGET="$(PC_PREFIX)$(DBG_PREFIX)$(APP_PREFIX)" -e CC=$(PC_CC) -e CFLAGS="$(PC_CFLAGS)" -e ARCH=$(ARCH_PC) -e DEBUG=$(DEBUG_DBG) -e BUILD=$(BUILD_APP)
#target for pc kernel builds with debug enabled.
PC_STRING = -e TARGET="$(PC_PREFIX)$(DBG_PREFIX)$(KERN_PREFIX)" -e CC=$(PC_CC) -e CFLAGS="$(PC_CFLAGS)" -e ARCH=$(ARCH_PC) -e DEBUG=$(DEBUG_DBG) -e BUILD=$(BUILD_KERNEL)
#target for avr kenrel builds with debug enabled.
AVR_STRING = -e TARGET="$(AVR_PREFIX)$(DBG_PREFIX)$(KERN_PREFIX)" -e CC=$(AVR_CC) -e CFLAGS="$(AVR_CFLAGS)" -e ARCH=$(ARCH_AVR) -e DEBUG=$(DEBUG_DBG) -e BUILD=$(BUILD_KERNEL)
#target for pc kernel without debug
PC_FRE_STRING = -e TARGET="$(PC_PREFIX)$(FRE_PREFIX)$(KERN_PREFIX)" -e CC=$(PC_CC) -e CFLAGS="$(PC_CFLAGS)" -e ARCH=$(ARCH_PC) -e DEBUG=$(DEBUG_FRE) -e BUILD=$(BUILD_KERNEL)
#target for avr kernel without debug
AVR_FRE_STRING = -e TARGET="$(AVR_PREFIX)$(FRE_PREFIX)$(KERN_PREFIX)" -e CC=$(AVR_CC) -e CFLAGS="$(AVR_CFLAGS)" -e ARCH=$(ARCH_AVR) -e DEBUG=$(DEBUG_FRE) -e BUILD=$(BUILD_KERNEL)
#############################################
#build rules
%.o: 
	$(CC) $(CFLAGS) -D $(BUILD) -o $@ -c $<

%.out:
	$(CC) $(CFLAGS) -D $(BUILD) -o $@ $^

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
	rm -f $(DIR)/core*
	rm -f $(DIR)/*~
	rm -f $(DIR)/*.hex
	rm -f $(DIR)/*.elf
	rm -f $(DIR)/*.aws

###########################
# tags 
#
ctags: 
	ctags --fields=+S -R .

#########################
