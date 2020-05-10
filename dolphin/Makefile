#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** all ***************)
build:
	$(info   *************** build ***************)
	$(MAKE) -C $@

check: build
	$(info   *************** check ***************)

.PHONY: all build
