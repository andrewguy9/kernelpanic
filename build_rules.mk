#prefix
PC_PREFIX = pc
AVR_PREFIX = avr
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
AVR_BUILD = AVR_BUILD
PC_BUILD = PC_BUILD

#############################################

#BUILD = $(PC_BUILD)
#CFLAGS = $(PC_CFLAGS)
#BUILD = $(PC_BUILD)
#CC = $(PC_CC)

#############################################
#combined strings
PC_STRING = -e TARGET=$(PC_PREFIX) -e CC=$(PC_CC) -e -e BUILD=$(PC_BUILD) CFLAGS="$(PC_CFLAGS)"

AVR_STRING = -e TARGET=$(AVR_PREFIX) -e CC=$(AVR_CC) -e BUILD=$(AVR_BUILD) -e CFLAGS="$(AVR_CFLAGS)"
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
.phony: 
cleandir:
	rm -f $(@D)/*.out
	rm -f $(@D)/*.o
	rm -f $(@D)/core*
	rm -f $(@D)/*~
	rm -f $(@D)/*.hex
	rm -f $(@D)/*.elf
	rm -f $(@D)/tags

###########################
# tags 
#
ctags: 
	ctags -R .

#########################
