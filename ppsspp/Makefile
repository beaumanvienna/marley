#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build
CORES = $(shell grep -c ^processor /proc/cpuinfo )

all: build
	$(info   *************** ppsspp all end ***************)
build:
	$(info   *************** ppsspp build ***************)
	$(MAKE) -C $@ -j$(CORES)

clean:
	$(info   *************** ppsspp clean ***************)
	$(MAKE) -C build $@

check: build
	$(info   *************** ppsspp check ***************)

.PHONY: all build
