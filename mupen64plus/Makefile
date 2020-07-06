#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = Source

all: Source
	$(info   *************** mupen all end ***************)
Source:
	$(info   *************** mupen Source ***************)
	$(MAKE) -C $@ -j8

clean:
	$(info   *************** mupen clean ***************)
	$(MAKE) -C Source $@

check: Source
	$(info   *************** mupen check ***************)

.PHONY: all Source
