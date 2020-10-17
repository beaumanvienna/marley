#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** ppsspp all end ***************)
install:
	$(info   *************** install checkpoint ***************)
build:
	$(info   *************** ppsspp build ***************)
	$(MAKE) -C $@

clean:
	$(info   *************** ppsspp clean ***************)
	$(MAKE) -C build $@

check: build
	$(info   *************** ppsspp check ***************)

.PHONY: all build
