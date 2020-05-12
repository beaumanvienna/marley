#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = Source

all: Source
	$(info   *************** mupen top level all ***************)
Source:
	$(info   *************** mupen top level Source ***************)
	$(MAKE) -C $@

clean:
	$(info   *************** mupen top level clean ***************)
	$(MAKE) -C Source $@

check: Source
	$(info   *************** mupen top level check ***************)

.PHONY: all Source
