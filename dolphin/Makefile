#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** dolhpin all ***************)
build:
	$(info   *************** dolhpin build ***************)
	$(MAKE) -C $@

check: build
	$(info   *************** dolhpin check ***************)

.PHONY: all build
