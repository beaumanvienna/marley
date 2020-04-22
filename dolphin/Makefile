#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS := build

all: build
build:
	$(MAKE) -C $@

.PHONY: all build
