#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** dolhpin all end ***************)
install:
	$(info   *************** install checkpoint ***************)
build:
	$(info   *************** dolhpin build ***************)
	$(MAKE) -C $@

check: build
	$(info   *************** dolhpin check ***************)

clean:
	$(info   *************** dolhpin clean ***************)
	$(MAKE) -C build $@

.PHONY: all build
