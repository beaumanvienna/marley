#this file is required to chain the autoconf run in ../../ with mupen64plus
SUBDIRS = audio-sdl core input-sdl rsp-hle video-glide64mk2 ui-console

CORES = $(shell cat .makeflags )
all: audio-sdl core input-sdl rsp-hle video-glide64mk2 ui-console
	$(info   *************** mupen64plus all end ***************)
	
clean:
	$(info   *************** mupen Source/ clean ***************)
	$(MAKE) -C audio-sdl $@
	$(MAKE) -C core $@
	$(MAKE) -C input-sdl $@
	$(MAKE) -C rsp-hle $@
	$(MAKE) -C ui-console $@
	$(MAKE) -C video-glide64mk2 $@
	rm -rf .makeflags

audio-sdl: 
	$(info   *************** mupen64plus audio-sdl ***************)
	$(MAKE) -C $@ $(CORES)
	
core: 
	$(info   *************** mupen64plus core ***************)
	$(MAKE) -C $@ $(CORES)

input-sdl: 
	$(info   *************** mupen64plus input-sdl ***************)
	$(MAKE) -C $@ $(CORES)
	
rsp-hle: 
	$(info   *************** mupen64plus rsp-hle ***************)
	$(MAKE) -C $@ $(CORES)
	
video-glide64mk2: 
	$(info   *************** mupen64plus video-glide64mk2 ***************)
	$(MAKE) -C $@ $(CORES)

ui-console: 
	$(info   *************** mupen64plus ui-console ***************)
	$(MAKE) -C $@ $(CORES)

check:
	$(info   *************** mupen64plus check ***************)

.PHONY: all audio-sdl core input-sdl rsp-hle video-glide64mk2 ui-console 
