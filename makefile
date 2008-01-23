all: umouse tests

include build_rules.mk

.PHONY: all tests umouse board motortest clean 

tests:
	make $(PC_STRING) -f makefile.main tests
umouse:
	make $(AVR_STRING) -f makefile.main umouse
board:
	make $(AVR_STRING) -f makefile.main umouse.hex
	avrdude -p m128 -U f:w:umouse.hex

clean: 
	make -e DIR=. cleandir
	make -e DIR=utils cleandir
	make -e DIR=kernel cleandir
	make -e DIR=test cleandir
	make -e DIR=umouse cleandir
