.PHONY: all tests avr avr_fre pc pc_fre clean

all: tests avr avr_fre pc pc_fre

include build_rules.mk

tests:
	$(MAKE_PROG) $(TEST_STRING) -f makefile.main tests
avr:
	$(MAKE_PROG) $(AVR_STRING) -f makefile.main umouse
avr_fre:
	$(MAKE_PROG) $(AVR_FRE_STRING) -f makefile.main umouse
pc:
	$(MAKE_PROG) $(PC_STRING) -f makefile.main umouse
pc_fre:
	$(MAKE_PROG) $(PC_FRE_STRING) -f makefile.main umouse

clean:
	$(MAKE_PROG) -e DIR=. cleandir
	$(MAKE_PROG) -e DIR=utils cleandir
	$(MAKE_PROG) -e DIR=kernel cleandir
	$(MAKE_PROG) -e DIR=test cleandir
	$(MAKE_PROG) -e DIR=umouse cleandir
