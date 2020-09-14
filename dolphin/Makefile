#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build
CORES = $(shell grep -c ^processor /proc/cpuinfo )

all: build
	$(info   *************** dolhpin all end ***************)
build:
	$(info   *************** dolhpin build ***************)
	$(MAKE) -C $@ -j$(CORES)

check: build
	$(info   *************** dolhpin check ***************)

clean:
	$(info   *************** dolhpin clean ***************)
	$(MAKE) -C build $@

.PHONY: all build
