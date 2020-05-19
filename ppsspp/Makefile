#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** ppsspp all ***************)
build:
	$(info   *************** ppsspp build ***************)
	$(MAKE) -C $@

check: build
	$(info   *************** ppsspp check ***************)

.PHONY: all build
