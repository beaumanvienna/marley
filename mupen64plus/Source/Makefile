#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = Source

all: Source
	$(info   *************** all ***************)
build:
	$(info   *************** build ***************)
	$(MAKE) -C $@

check: Source
	$(info   *************** check ***************)

.PHONY: all Source
