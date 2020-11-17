// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "ppsspp_config.h"

#include <cstring>
#include <cstdlib>

#include "Common/Common.h"
#include "Common/Log.h"
#include "Common/MemoryUtil.h"
#include "Common/StringUtils.h"
#include "Common/SysError.h"

#include <unistd.h>

static int hint_location;

#define MEM_PAGE_SIZE (getpagesize())
#define MEM_PAGE_MASK ((MEM_PAGE_SIZE)-1)
#define ppsspp_round_page(x) ((((uintptr_t)(x)) + MEM_PAGE_MASK) & ~(MEM_PAGE_MASK))

static uint32_t ConvertProtFlagsUnix(uint32_t flags) {
	uint32_t protect = 0;
	if (flags & MEM_PROT_READ)
		protect |= PROT_READ;
	if (flags & MEM_PROT_WRITE)
		protect |= PROT_WRITE;
	if (flags & MEM_PROT_EXEC)
		protect |= PROT_EXEC;
	return protect;
}

// This is purposely not a full wrapper for virtualalloc/mmap, but it
// provides exactly the primitive operations that PPSSPP needs.
void *AllocateExecutableMemory(size_t size) {

	static char *map_hint = 0;

	// Try to request one that is close to our memory location if we're in high memory.
	// We use a dummy global variable to give us a good location to start from.
	if (!map_hint) {
		if ((uintptr_t) &hint_location > 0xFFFFFFFFULL)
			map_hint = (char*)ppsspp_round_page(&hint_location) - 0x20000000; // 0.5gb lower than our approximate location
		else
			map_hint = (char*)0x20000000; // 0.5GB mark in memory
	} else if ((uintptr_t) map_hint > 0xFFFFFFFFULL) {
		map_hint -= ppsspp_round_page(size); /* round down to the next page if we're in high memory */
	}


	int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
	if (PlatformIsWXExclusive())
		prot = PROT_READ | PROT_WRITE;  // POST_EXEC is added later in this case.

	void* ptr = mmap(map_hint, size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);

	static const void *failed_result = MAP_FAILED;

	if (ptr == failed_result) {
		ptr = nullptr;
		printf("Failed to allocate executable memory (%d) errno=%d", (int)size, errno);
	}
	else if ((uintptr_t)map_hint <= 0xFFFFFFFF) {
		// Round up if we're below 32-bit mark, probably allocating sequentially.
		map_hint += ppsspp_round_page(size);

		// If we moved ahead too far, skip backwards and recalculate.
		// When we free, we keep moving forward and eventually move too far.
		if ((uintptr_t)map_hint - (uintptr_t) &hint_location >= 0x70000000) {
			map_hint = 0;
		}
	}
	return ptr;
}

void *AllocateMemoryPages(size_t size, uint32_t memProtFlags) {
	size = ppsspp_round_page(size);

	uint32_t protect = ConvertProtFlagsUnix(memProtFlags);
	void *ptr = mmap(0, size, protect, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (ptr == MAP_FAILED) {
		printf("Failed to allocate raw memory pages: errno=%d", errno);
		return nullptr;
	}

	// printf("Mapped memory at %p (size %ld)\n", ptr,
	//	(unsigned long)size);
	return ptr;
}

void *AllocateAlignedMemory(size_t size, size_t alignment) {


	void* ptr = NULL;
	if (posix_memalign(&ptr, alignment, size) != 0) {
		ptr = nullptr;
	}
	_assert_msg_(ptr != nullptr, "Failed to allocate aligned memory");
	return ptr;
}

void FreeMemoryPages(void *ptr, size_t size) {
	if (!ptr)
		return;
	uintptr_t page_size = GetMemoryProtectPageSize();
	size = (size + page_size - 1) & (~(page_size - 1));

	munmap(ptr, size);

}

void FreeAlignedMemory(void* ptr) {
	if (!ptr)
		return;

	free(ptr);

}

bool PlatformIsWXExclusive() {
	// Needed on platforms that disable W^X pages for security. Even without block linking, still should be much faster than IR JIT.
	// This might also come in useful for UWP (Universal Windows Platform) if I'm understanding things correctly.
	return false;
}

bool ProtectMemoryPages(const void* ptr, size_t size, uint32_t memProtFlags) {
	printf("ProtectMemoryPages: %p (%d) : r%d w%d x%d", ptr, (int)size,
			(memProtFlags & MEM_PROT_READ) != 0, (memProtFlags & MEM_PROT_WRITE) != 0, (memProtFlags & MEM_PROT_EXEC) != 0);

	if (PlatformIsWXExclusive()) {
		if ((memProtFlags & (MEM_PROT_WRITE | MEM_PROT_EXEC)) == (MEM_PROT_WRITE | MEM_PROT_EXEC)) {
			_assert_msg_(false, "Bad memory protect flags %d: W^X is in effect, can't both write and exec", memProtFlags);
		}
	}
	// Note - VirtualProtect will affect the full pages containing the requested range.
	// mprotect does not seem to, at least not on Android unless I made a mistake somewhere, so we manually round.

	uint32_t protect = ConvertProtFlagsUnix(memProtFlags);
	uintptr_t page_size = GetMemoryProtectPageSize();

	uintptr_t start = (uintptr_t)ptr;
	uintptr_t end = (uintptr_t)ptr + size;
	start &= ~(page_size - 1);
	end = (end + page_size - 1) & ~(page_size - 1);
	int retval = mprotect((void *)start, end - start, protect);
	if (retval != 0) {
		printf("mprotect failed (%p)! errno=%d (%s)", (void *)start, errno, strerror(errno));
		return false;
	}
	return true;

}

int GetMemoryProtectPageSize() {
    
	return MEM_PAGE_SIZE;
}
