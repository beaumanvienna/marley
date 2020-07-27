// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include "Core/HLE/HLE.h"
#include "Core/HLE/FunctionWrappers.h"
#include "Core/HLE/sceMd5.h"
#include "Core/MemMap.h"
#include "Core/Reporting.h"
#include "Common/Crypto/md5.h"
#include "Common/Crypto/sha1.h"

#ifdef USE_CRT_DBG
#undef new
#endif

// Not really sure where these belong - is it worth giving them their own file?
u32 sceKernelUtilsMt19937Init(u32 ctx, u32 seed) {
	DEBUG_LOG(HLE, "sceKernelUtilsMt19937Init(%08x, %08x)", ctx, seed);
	if (!PMemory::IsValidAddress(ctx))
		return -1;
	void *ptr = PMemory::GetPointer(ctx);
	// This is made to match the memory layout of a PSP MT structure exactly.
	// Let's just construct it in place with placement new. Elite C++ hackery FTW.
	new (ptr) MersenneTwister(seed);
	return 0;
}

u32 sceKernelUtilsMt19937UInt(u32 ctx) {
	VERBOSE_LOG(HLE, "sceKernelUtilsMt19937UInt(%08x)", ctx);
	if (!PMemory::IsValidAddress(ctx))
		return -1;
	MersenneTwister *mt = (MersenneTwister *)PMemory::GetPointer(ctx);
	return mt->R32();
}

// TODO: This MD5 stuff needs tests!

static md5_context md5_ctx;

static int sceMd5Digest(u32 dataAddr, u32 len, u32 digestAddr) {
	DEBUG_LOG(HLE, "sceMd5Digest(%08x, %d, %08x)", dataAddr, len, digestAddr);

	if (!PMemory::IsValidAddress(dataAddr) || !PMemory::IsValidAddress(digestAddr))
		return -1;

	md5(PMemory::GetPointer(dataAddr), (int)len, PMemory::GetPointer(digestAddr));
	return 0;
}

static int sceMd5BlockInit(u32 ctxAddr) {
	DEBUG_LOG(HLE, "sceMd5BlockInit(%08x)", ctxAddr);
	if (!PMemory::IsValidAddress(ctxAddr))
		return -1;

	// TODO: Until I know how large a context is, we just go all lazy and use a global context,
	// which will work just fine unless games do several MD5 concurrently.

	md5_starts(&md5_ctx);
	return 0;
}

static int sceMd5BlockUpdate(u32 ctxAddr, u32 dataPtr, u32 len) {
	DEBUG_LOG(HLE, "sceMd5BlockUpdate(%08x, %08x, %d)", ctxAddr, dataPtr, len);
	if (!PMemory::IsValidAddress(ctxAddr) || !PMemory::IsValidAddress(dataPtr))
		return -1;
	
	md5_update(&md5_ctx, PMemory::GetPointer(dataPtr), (int)len);
	return 0;
}

static int sceMd5BlockResult(u32 ctxAddr, u32 digestAddr) {
	DEBUG_LOG(HLE, "sceMd5BlockResult(%08x, %08x)", ctxAddr, digestAddr);
	if (!PMemory::IsValidAddress(ctxAddr) || !PMemory::IsValidAddress(digestAddr))
		return -1;

	Pmd5_finish(&md5_ctx, PMemory::GetPointer(digestAddr));
	return 0;
}

int sceKernelUtilsMd5Digest(u32 dataAddr, int len, u32 digestAddr) {
	DEBUG_LOG(HLE, "sceKernelUtilsMd5Digest(%08x, %d, %08x)", dataAddr, len, digestAddr);

	if (!PMemory::IsValidAddress(dataAddr) || !PMemory::IsValidAddress(digestAddr))
		return -1;

	md5(PMemory::GetPointer(dataAddr), (int)len, PMemory::GetPointer(digestAddr));
	return 0;
}

int sceKernelUtilsMd5BlockInit(u32 ctxAddr) {
	DEBUG_LOG(HLE, "sceKernelUtilsMd5BlockInit(%08x)", ctxAddr);
	if (!PMemory::IsValidAddress(ctxAddr))
		return -1;

	// TODO: Until I know how large a context is, we just go all lazy and use a global context,
	// which will work just fine unless games do several MD5 concurrently.

	md5_starts(&md5_ctx);
	return 0;
}

int sceKernelUtilsMd5BlockUpdate(u32 ctxAddr, u32 dataPtr, int len) {
	DEBUG_LOG(HLE, "sceKernelUtilsMd5BlockUpdate(%08x, %08x, %d)", ctxAddr, dataPtr, len);
	if (!PMemory::IsValidAddress(ctxAddr) || !PMemory::IsValidAddress(dataPtr))
		return -1;

	md5_update(&md5_ctx, PMemory::GetPointer(dataPtr), (int)len);
	return 0;
}

int sceKernelUtilsMd5BlockResult(u32 ctxAddr, u32 digestAddr) {
	DEBUG_LOG(HLE, "sceKernelUtilsMd5BlockResult(%08x, %08x)", ctxAddr, digestAddr);
	if (!PMemory::IsValidAddress(ctxAddr) || !PMemory::IsValidAddress(digestAddr))
		return -1;

	Pmd5_finish(&md5_ctx, PMemory::GetPointer(digestAddr));
	return 0;
}


static sha1_context sha1_ctx;

int sceKernelUtilsSha1Digest(u32 dataAddr, int len, u32 digestAddr) {
	DEBUG_LOG(HLE, "sceKernelUtilsSha1Digest(%08x, %d, %08x)", dataAddr, len, digestAddr);

	if (!PMemory::IsValidAddress(dataAddr) || !PMemory::IsValidAddress(digestAddr))
		return -1;

	sha1(PMemory::GetPointer(dataAddr), (int)len, PMemory::GetPointer(digestAddr));
	return 0;
}

int sceKernelUtilsSha1BlockInit(u32 ctxAddr) {
	DEBUG_LOG(HLE, "sceKernelUtilsSha1BlockInit(%08x)", ctxAddr);
	if (!PMemory::IsValidAddress(ctxAddr))
		return -1;

	// TODO: Until I know how large a context is, we just go all lazy and use a global context,
	// which will work just fine unless games do several MD5 concurrently.

	sha1_starts(&sha1_ctx);

	return 0;
}

int sceKernelUtilsSha1BlockUpdate(u32 ctxAddr, u32 dataAddr, int len) {
	DEBUG_LOG(HLE, "sceKernelUtilsSha1BlockUpdate(%08x, %08x, %d)", ctxAddr, dataAddr, len);
	if (!PMemory::IsValidAddress(ctxAddr) || !PMemory::IsValidAddress(dataAddr))
		return -1;

	sha1_update(&sha1_ctx, PMemory::GetPointer(dataAddr), (int)len);
	return 0;
}

int sceKernelUtilsSha1BlockResult(u32 ctxAddr, u32 digestAddr) {
	DEBUG_LOG(HLE, "sceKernelUtilsSha1BlockResult(%08x, %08x)", ctxAddr, digestAddr);
	if (!PMemory::IsValidAddress(ctxAddr) || !PMemory::IsValidAddress(digestAddr))
		return -1;

	sha1_finish(&sha1_ctx, PMemory::GetPointer(digestAddr));
	return 0;
}


const HLEFunction sceMd5[] = {
	{0X19884A15, &WrapI_U<sceMd5BlockInit>,          "sceMd5BlockInit",   'i', "x"  },
	{0XA30206C2, &WrapI_UUU<sceMd5BlockUpdate>,      "sceMd5BlockUpdate", 'i', "xxx"},
	{0X4876AFFF, &WrapI_UU<sceMd5BlockResult>,       "sceMd5BlockResult", 'i', "xx" },
	{0X98E31A9E, &WrapI_UUU<sceMd5Digest>,           "sceMd5Digest",      'i', "xxx"},
};

void Register_sceMd5() {
	RegisterModule("sceMd5", ARRAY_SIZE(sceMd5), sceMd5);
}
