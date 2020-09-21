#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** pcsx2 all end ***************)
install:
	$(info   *************** install checkpoint ***************)
build:
	$(info   *************** pcsx2 build ***************)
	$(MAKE) -C $@

clean:
	$(info   *************** pcsx2 clean ***************)
	$(MAKE) -C build $@

check: build
	$(info   *************** pcsx2 check ***************)

.PHONY: all build
