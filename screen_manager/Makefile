#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build

all: build
	$(info   *************** screen_manager all end ***************)
install:
	$(info   *************** install checkpoint ***************)
build:
	$(info   *************** screen_manager build ***************)
	$(MAKE) -C $@

clean:
	$(info   *************** screen_manager clean ***************)
	rm -rf build

check: build
	$(info   *************** screen_manager check ***************)

.PHONY: all build
