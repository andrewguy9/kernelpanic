.PHONY: all tests avr avr_fre pc pc_fre clean

all: tests avr avr_fre pc pc_fre

include build_rules.mk

tests:
	make $(TEST_STRING) -f makefile.main tests
avr:
	make $(AVR_STRING) -f makefile.main umouse
avr_fre:
	make $(AVR_FRE_STRING) -f makefile.main umouse
pc:
	make $(PC_STRING) -f makefile.main umouse
pc_fre: 
	make $(PC_FRE_STRING) -f makefile.main umouse

clean: 
	make -e DIR=. cleandir
	make -e DIR=utils cleandir
	make -e DIR=kernel cleandir
	make -e DIR=test cleandir
	make -e DIR=umouse cleandir
