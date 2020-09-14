#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = build
CORES = $(shell echo $(nproc) )

all: build
	$(info   *************** pcsx2 all end ***************)
build:
	$(info   *************** pcsx2 build ***************)
	$(MAKE) -C $@ -j$(CORES)

clean:
	$(info   *************** pcsx2 clean ***************)
	$(MAKE) -C build $@

check: build
	$(info   *************** pcsx2 check ***************)

.PHONY: all build
