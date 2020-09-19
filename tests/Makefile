# Makefile for unit tests

all: 

clean:
	$(info   *************** tests clean ***************)
	$(MAKE) -C pcsx2 $@
	$(MAKE) -C ppsspp $@
	$(MAKE) -C mupen64plus $@
	$(MAKE) -C mednafen $@
	$(MAKE) -C dolphin $@

install:
	$(info   *************** install checkpoint ***************)

unit_tests:
	$(info   *************** make all unit tests ***************)
	$(MAKE) -C pcsx2 all
	$(MAKE) -C ppsspp all
	$(MAKE) -C mupen64plus all
	$(MAKE) -C mednafen all
	$(MAKE) -C dolphin all

check: all

.PHONY: all unit_tests clean
