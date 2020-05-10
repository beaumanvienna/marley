#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = audio-sdl  core  input-sdl  rsp-hle  ui-console  video-glide64mk2  video-rice

all: audio-sdl  core  input-sdl  rsp-hle  ui-console  video-glide64mk2  video-rice
	$(info   *************** mupen64plus all ***************)
	
audio-sdl:
	$(info   *************** mupen64plus audio-sdl ***************)
	$(MAKE) -C $@
	
core:
	$(info   *************** mupen64plus core ***************)
	$(MAKE) -C $@

input-sdl:
	$(info   *************** mupen64plus input-sdl ***************)
	$(MAKE) -C $@
	
rsp-hle:
	$(info   *************** mupen64plus rsp-hle ***************)
	$(MAKE) -C $@
	
ui-console:
	$(info   *************** mupen64plus ui-console ***************)
	$(MAKE) -C $@
	
video-glide64mk2:
	$(info   *************** mupen64plus video-glide64mk2 ***************)
	$(MAKE) -C $@

video-rice:
	$(info   *************** mupen64plus video-rice ***************)
	$(MAKE) -C $@


check: audio-sdl  core  input-sdl  rsp-hle  ui-console  video-glide64mk2  video-rice
	$(info   *************** mupen64plus check ***************)

.PHONY: all audio-sdl  core  input-sdl  rsp-hle  ui-console  video-glide64mk2  video-rice
