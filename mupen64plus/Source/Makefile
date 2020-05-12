#this file is required to chain the autoconf run in ../ with the cmake run in build/
SUBDIRS = audio-sdl core input-sdl rsp-hle video-glide64mk2 video-rice ui-console

all: ui-console ui-console/mupen64plus
	$(info   *************** mupen64plus all ***************)
	
clean:
	$(info   *************** mupen Source/ clean ***************)
	$(MAKE) -C audio-sdl $@
	$(MAKE) -C core $@
	$(MAKE) -C input-sdl $@
	$(MAKE) -C rsp-hle $@
	$(MAKE) -C ui-console $@
	$(MAKE) -C video-glide64mk2 $@
	$(MAKE) -C video-rice $@
	
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
	
video-glide64mk2:
	$(info   *************** mupen64plus video-glide64mk2 ***************)
	$(MAKE) -C $@

video-rice:
	$(info   *************** mupen64plus video-rice ***************)
	$(MAKE) -C $@

ui-console: audio-sdl core input-sdl rsp-hle video-glide64mk2 video-rice
	$(info   *************** mupen64plus ui-console ***************)
	$(MAKE) -C $@

ui-console/mupen64plus: audio-sdl/mupen64plus-audio-sdl.a core/libmupen64plus.a input-sdl/mupen64plus-input-sdl.a rsp-hle/mupen64plus-rsp-hle.a video-glide64mk2/mupen64plus-video-glide64mk2.a
	$(info   *************** mupen64plus ui-console ***************)
	rm -f ui-console/mupen64plus
	$(MAKE) -C ui-console all

check:
	$(info   *************** mupen64plus check ***************)

.PHONY: all audio-sdl  core  input-sdl  rsp-hle video-glide64mk2  video-rice ui-console 
