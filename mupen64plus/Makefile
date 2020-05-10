#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = Source

all: Source
	$(info   *************** all ***************)
Source:
	$(info   *************** Source ***************)
	$(MAKE) -C $@

check: Source
	$(info   *************** check ***************)

.PHONY: all Source
