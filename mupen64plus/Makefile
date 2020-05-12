#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = Source

all: Source
	$(info   *************** mupen top level all end ***************)
Source:
	$(info   *************** mupen top level Source end ***************)
	$(MAKE) -C $@

clean:
	$(info   *************** mupen top level clean ***************)
	$(MAKE) -C Source $@

check: Source
	$(info   *************** mupen top level check ***************)

.PHONY: all Source
