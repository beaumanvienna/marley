// Copyright (c) 2017- PPSSPP Project.

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

#include <algorithm>
#include <cstring>
#include <functional>
#include <set>
#include <vector>
#include <snappy-c.h>
#include "base/stringutil.h"
#include "Common/Common.h"
#include "Common/FileUtil.h"
#include "Common/Log.h"
#include "Core/Core.h"
#include "Core/ELF/ParamSFO.h"
#include "Core/HLE/sceDisplay.h"
#include "Core/MemMap.h"
#include "Core/System.h"
#include "GPU/GPUInterface.h"
#include "GPU/GPUState.h"
#include "GPU/ge_constants.h"
#include "GPU/Common/TextureDecoder.h"
#include "GPU/Common/VertexDecoderCommon.h"
#include "GPU/Debugger/Record.h"
#include "GPU/Debugger/RecordFormat.h"

namespace GPURecord {

static bool active = false;
static bool nextFrame = false;
static int flipLastAction = -1;
static std::function<void(const std::string &)> writeCallback;

static std::vector<u8> pushbuf;
static std::vector<Command> commands;
static std::vector<u32> lastRegisters;
static std::vector<u32> lastTextures;
static std::set<u32> lastRenderTargets;

static void FlushRegisters() {
	if (!lastRegisters.empty()) {
		Command last{CommandType::REGISTERS};
		last.ptr = (u32)pushbuf.size();
		last.sz = (u32)(lastRegisters.size() * sizeof(u32));
		pushbuf.resize(pushbuf.size() + last.sz);
		memcpy(pushbuf.data() + last.ptr, lastRegisters.data(), last.sz);
		lastRegisters.clear();

		commands.push_back(last);
	}
}

static std::string GenRecordingFilename() {
	const std::string dumpDir = GetSysDirectory(DIRECTORY_DUMP);
	const std::string prefix = dumpDir + g_paramSFO.GetDiscID();

	PFile::CreateFullPath(dumpDir);

	for (int n = 1; n < 10000; ++n) {
		std::string filename = PStringFromFormat("%s_%04d.ppdmp", prefix.c_str(), n);
		if (!PFile::Exists(filename)) {
			return filename;
		}
	}

	return PStringFromFormat("%s_%04d.ppdmp", prefix.c_str(), 9999);
}

static void BeginRecording() {
	active = true;
	nextFrame = false;
	lastTextures.clear();
	lastRenderTargets.clear();
	flipLastAction = gpuStats.numFlips;

	u32 ptr = (u32)pushbuf.size();
	u32 sz = 512 * 4;
	pushbuf.resize(pushbuf.size() + sz);
	gstate.Save((u32_le *)(pushbuf.data() + ptr));

	commands.push_back({CommandType::INIT, sz, ptr});
}

static void WriteCompressed(FILE *fp, const void *p, size_t sz) {
	size_t compressed_size = snappy_max_compressed_length(sz);
	u8 *compressed = new u8[compressed_size];
	snappy_compress((const char *)p, sz, (char *)compressed, &compressed_size);

	u32 write_size = (u32)compressed_size;
	fwrite(&write_size, sizeof(write_size), 1, fp);
	fwrite(compressed, compressed_size, 1, fp);

	delete [] compressed;
}

static std::string WriteRecording() {
	FlushRegisters();

	const std::string filename = GenRecordingFilename();

	NOTICE_LOG(G3D, "Recording filename: %s", filename.c_str());

	FILE *fp = PFile::OpenCFile(filename, "wb");
	fwrite(HEADER, 8, 1, fp);
	fwrite(&VERSION, sizeof(VERSION), 1, fp);

	u32 sz = (u32)commands.size();
	fwrite(&sz, sizeof(sz), 1, fp);
	u32 bufsz = (u32)pushbuf.size();
	fwrite(&bufsz, sizeof(bufsz), 1, fp);

	WriteCompressed(fp, commands.data(), commands.size() * sizeof(Command));
	WriteCompressed(fp, pushbuf.data(), bufsz);

	fclose(fp);

	return filename;
}

static void GetVertDataSizes(int vcount, const void *indices, u32 &vbytes, u32 &ibytes) {
	VertexDecoder vdec;
	VertexDecoderOptions opts{};
	vdec.SetVertexType(gstate.vertType, opts);

	if (indices) {
		u16 lower = 0;
		u16 upper = 0;
		GetIndexBounds(indices, vcount, gstate.vertType, &lower, &upper);

		vbytes = (upper + 1) * vdec.VertexSize();
		u32 idx = gstate.vertType & GE_VTYPE_IDX_MASK;
		if (idx == GE_VTYPE_IDX_8BIT) {
			ibytes = vcount * sizeof(u8);
		} else if (idx == GE_VTYPE_IDX_16BIT) {
			ibytes = vcount * sizeof(u16);
		} else if (idx == GE_VTYPE_IDX_32BIT) {
			ibytes = vcount * sizeof(u32);
		}
	} else {
		vbytes = vcount * vdec.VertexSize();
	}
}

static const u8 *mymemmem(const u8 *haystack, size_t hlen, const u8 *needle, size_t nlen) {
	if (!nlen) {
		return nullptr;
	}

	const u8 *last_possible = haystack + hlen - nlen;
	int first = *needle;
	const u8 *p = haystack;
	while (p <= last_possible) {
		p = (const u8 *)memchr(p, first, last_possible - p + 1);
		if (!p) {
			return nullptr;
		}
		if (!memcmp(p, needle, nlen)) {
			return p;
		}

		p++;
	}

	return nullptr;
}

static Command EmitCommandWithRAM(CommandType t, const void *p, u32 sz) {
	FlushRegisters();

	Command cmd{t, sz, 0};

	if (sz) {
		// If at all possible, try to find it already in the buffer.
		const u8 *prev = nullptr;
		const size_t NEAR_WINDOW = std::max((int)sz * 2, 1024 * 10);
		// Let's try nearby first... it will often be nearby.
		if (pushbuf.size() > NEAR_WINDOW) {
			prev = mymemmem(pushbuf.data() + pushbuf.size() - NEAR_WINDOW, NEAR_WINDOW, (const u8 *)p, sz);
		}
		if (!prev) {
			prev = mymemmem(pushbuf.data(), pushbuf.size(), (const u8 *)p, sz);
		}

		if (prev) {
			cmd.ptr = (u32)(prev - pushbuf.data());
		} else {
			cmd.ptr = (u32)pushbuf.size();
			int pad = 0;
			if (cmd.ptr & 0xF) {
				pad = 0x10 - (cmd.ptr & 0xF);
				cmd.ptr += pad;
			}
			pushbuf.resize(pushbuf.size() + sz + pad);
			if (pad) {
				memset(pushbuf.data() + cmd.ptr - pad, 0, pad);
			}
			memcpy(pushbuf.data() + cmd.ptr, p, sz);
		}
	}

	commands.push_back(cmd);

	return cmd;
}

static void EmitTextureData(int level, u32 texaddr) {
	GETextureFormat format = gstate.getTextureFormat();
	int w = gstate.getTextureWidth(level);
	int h = gstate.getTextureHeight(level);
	int bufw = GetTextureBufw(level, texaddr, format);
	int extraw = w > bufw ? w - bufw : 0;
	u32 sizeInRAM = (textureBitsPerPixel[format] * (bufw * h + extraw)) / 8;
	const bool isTarget = lastRenderTargets.find(texaddr) != lastRenderTargets.end();

	CommandType type = CommandType((int)CommandType::TEXTURE0 + level);
	const u8 *p = Memory_P::GetPointerUnchecked(texaddr);
	u32 bytes = Memory_P::ValidSize(texaddr, sizeInRAM);
	std::vector<u8> framebufData;

	if (Memory_P::IsVRAMAddress(texaddr)) {
		struct FramebufData {
			u32 addr;
			int bufw;
			u32 flags;
			u32 pad;
		};

		// The isTarget flag is mostly used for replay of dumps on a PSP.
		u32 flags = isTarget ? 1 : 0;
		FramebufData framebuf{ texaddr, bufw, flags };
		framebufData.resize(sizeof(framebuf) + bytes);
		memcpy(&framebufData[0], &framebuf, sizeof(framebuf));
		memcpy(&framebufData[sizeof(framebuf)], p, bytes);
		p = &framebufData[0];

		// Okay, now we'll just emit this instead.
		type = CommandType((int)CommandType::FRAMEBUF0 + level);
		bytes += (u32)sizeof(framebuf);
	}

	if (bytes > 0) {
		FlushRegisters();

		// Dumps are huge - let's try to find this already emitted.
		for (u32 prevptr : lastTextures) {
			if (pushbuf.size() < prevptr + bytes) {
				continue;
			}

			if (memcmp(pushbuf.data() + prevptr, p, bytes) == 0) {
				commands.push_back({type, bytes, prevptr});
				// Okay, that was easy.  Bail out.
				return;
			}
		}

		// Not there, gotta emit anew.
		Command cmd = EmitCommandWithRAM(type, p, bytes);
		lastTextures.push_back(cmd.ptr);
	}
}

static void FlushPrimState(int vcount) {
	// TODO: Eventually, how do we handle texturing from framebuf/zbuf?
	// TODO: Do we need to preload color/depth/stencil (in case from last frame)?

	lastRenderTargets.insert(PSP_GetVidMemBase() | gstate.getFrameBufRawAddress());
	lastRenderTargets.insert(PSP_GetVidMemBase() | gstate.getDepthBufRawAddress());

	// We re-flush textures always in case the game changed them... kinda expensive.
	// TODO: Dirty textures on transfer/stall/etc. somehow?
	// TODO: Or maybe de-dup by validating if it has changed?
	for (int level = 0; level < 8; ++level) {
		u32 texaddr = gstate.getTextureAddress(level);
		if (texaddr) {
			EmitTextureData(level, texaddr);
		}
	}

	const void *verts = Memory_P::GetPointer(gstate_c.vertexAddr);
	const void *indices = nullptr;
	if ((gstate.vertType & GE_VTYPE_IDX_MASK) != GE_VTYPE_IDX_NONE) {
		indices = Memory_P::GetPointer(gstate_c.indexAddr);
	}

	u32 ibytes = 0;
	u32 vbytes = 0;
	GetVertDataSizes(vcount, indices, vbytes, ibytes);

	if (indices && ibytes > 0) {
		EmitCommandWithRAM(CommandType::INDICES, indices, ibytes);
	}
	if (verts && vbytes > 0) {
		EmitCommandWithRAM(CommandType::VERTICES, verts, vbytes);
	}
}

static void EmitTransfer(u32 op) {
	FlushRegisters();

	// This may not make a lot of sense right now, unless it's to a framebuf...
	if (!Memory_P::IsVRAMAddress(gstate.getTransferDstAddress())) {
		// Skip, not VRAM, so can't affect drawing (we flush textures each prim.)
		return;
	}

	u32 srcBasePtr = gstate.getTransferSrcAddress();
	u32 srcStride = gstate.getTransferSrcStride();
	int srcX = gstate.getTransferSrcX();
	int srcY = gstate.getTransferSrcY();
	int width = gstate.getTransferWidth();
	int height = gstate.getTransferHeight();
	int bpp = gstate.getTransferBpp();

	u32 srcBytes = ((srcY + height - 1) * srcStride + (srcX + width)) * bpp;
	srcBytes = Memory_P::ValidSize(srcBasePtr, srcBytes);

	if (srcBytes != 0) {
		EmitCommandWithRAM(CommandType::TRANSFERSRC, Memory_P::GetPointerUnchecked(srcBasePtr), srcBytes);
	}

	lastRegisters.push_back(op);
}

static void EmitClut(u32 op) {
	u32 addr = gstate.getClutAddress();
	u32 bytes = (op & 0x3F) * 32;
	bytes = Memory_P::ValidSize(addr, bytes);

	if (bytes != 0) {
		EmitCommandWithRAM(CommandType::CLUT, Memory_P::GetPointerUnchecked(addr), bytes);
	}

	lastRegisters.push_back(op);
}

static void EmitPrim(u32 op) {
	FlushPrimState(op & 0x0000FFFF);

	lastRegisters.push_back(op);
}

static void EmitBezierSpline(u32 op) {
	int ucount = op & 0xFF;
	int vcount = (op >> 8) & 0xFF;
	FlushPrimState(ucount * vcount);

	lastRegisters.push_back(op);
}

bool IsActive() {
	return active;
}

bool IsActivePending() {
	return nextFrame || active;
}

bool Activate() {
	if (!nextFrame) {
		nextFrame = true;
		flipLastAction = gpuStats.numFlips;
		return true;
	}
	return false;
}

void SetCallback(const std::function<void(const std::string &)> callback) {
	writeCallback = callback;
}

static void FinishRecording() {
	// We're done - this was just to write the result out.
	std::string filename = WriteRecording();
	commands.clear();
	pushbuf.clear();

	NOTICE_LOG(SYSTEM, "Recording finished");
	active = false;
	flipLastAction = gpuStats.numFlips;

	if (writeCallback)
		writeCallback(filename);
	writeCallback = nullptr;
}

void NotifyCommand(u32 pc) {
	if (!active) {
		return;
	}

	const u32 op = Memory_P::PRead_U32(pc);
	const GECommand cmd = GECommand(op >> 24);

	switch (cmd) {
	case GE_CMD_VADDR:
	case GE_CMD_IADDR:
	case GE_CMD_JUMP:
	case GE_CMD_CALL:
	case GE_CMD_RET:
	case GE_CMD_END:
	case GE_CMD_SIGNAL:
	case GE_CMD_FINISH:
	case GE_CMD_BASE:
	case GE_CMD_OFFSETADDR:
	case GE_CMD_ORIGIN:
		// These just prepare future commands, and are flushed with those commands.
		// TODO: Maybe add a command just to log that these were hit?
		break;

	case GE_CMD_BOUNDINGBOX:
	case GE_CMD_BJUMP:
		// Since we record each command, this is theoretically not relevant.
		// TODO: Output a CommandType to validate this.
		break;

	case GE_CMD_PRIM:
		EmitPrim(op);
		break;

	case GE_CMD_BEZIER:
	case GE_CMD_SPLINE:
		EmitBezierSpline(op);
		break;

	case GE_CMD_LOADCLUT:
		EmitClut(op);
		break;

	case GE_CMD_TRANSFERSTART:
		EmitTransfer(op);
		break;

	default:
		lastRegisters.push_back(op);
		break;
	}
}

void NotifyMemcpy(u32 dest, u32 src, u32 sz) {
	if (!active) {
		return;
	}
	if (Memory_P::IsVRAMAddress(dest)) {
		FlushRegisters();
		Command cmd{CommandType::MEMCPYDEST, sizeof(dest), (u32)pushbuf.size()};
		pushbuf.resize(pushbuf.size() + sizeof(dest));
		memcpy(pushbuf.data() + cmd.ptr, &dest, sizeof(dest));

		sz = Memory_P::ValidSize(dest, sz);
		if (sz != 0) {
			EmitCommandWithRAM(CommandType::MEMCPYDATA, Memory_P::GetPointer(dest), sz);
		}
	}
}

void NotifyMemset(u32 dest, int v, u32 sz) {
	if (!active) {
		return;
	}
	struct MemsetCommand {
		u32 dest;
		int value;
		u32 sz;
	};

	if (Memory_P::IsVRAMAddress(dest)) {
		sz = Memory_P::ValidSize(dest, sz);
		MemsetCommand data{dest, v, sz};

		FlushRegisters();
		Command cmd{CommandType::MEMSET, sizeof(data), (u32)pushbuf.size()};
		pushbuf.resize(pushbuf.size() + sizeof(data));
		memcpy(pushbuf.data() + cmd.ptr, &data, sizeof(data));
	}
}

void NotifyUpload(u32 dest, u32 sz) {
	if (!active) {
		return;
	}
	NotifyMemcpy(dest, dest, sz);
}

void NotifyDisplay(u32 framebuf, int stride, int fmt) {
	bool writePending = false;
	if (active && !commands.empty()) {
		writePending = true;
	}
	if (nextFrame && (gstate_c.skipDrawReason & SKIPDRAW_SKIPFRAME) == 0) {
		NOTICE_LOG(SYSTEM, "Recording starting on display...");
		BeginRecording();
	}
	if (!active) {
		return;
	}

	struct DisplayBufData {
		PSPPointer<u8> topaddr;
		int linesize, pixelFormat;
	};

	DisplayBufData disp{ { framebuf }, stride, fmt };

	FlushRegisters();
	u32 ptr = (u32)pushbuf.size();
	u32 sz = (u32)sizeof(disp);
	pushbuf.resize(pushbuf.size() + sz);
	memcpy(pushbuf.data() + ptr, &disp, sz);

	commands.push_back({ CommandType::DISPLAY, sz, ptr });

	if (writePending) {
		NOTICE_LOG(SYSTEM, "Recording complete on display");
		FinishRecording();
	}
}

void NotifyFrame() {
	const bool noDisplayAction = flipLastAction + 4 < gpuStats.numFlips;
	// We do this only to catch things that don't call NotifyDisplay.
	if (active && !commands.empty() && noDisplayAction) {
		NOTICE_LOG(SYSTEM, "Recording complete on frame");

		struct DisplayBufData {
			PSPPointer<u8> topaddr;
			u32 linesize, pixelFormat;
		};

		DisplayBufData disp;
		__DisplayGetFramebuf(&disp.topaddr, &disp.linesize, &disp.pixelFormat, 0);

		FlushRegisters();
		u32 ptr = (u32)pushbuf.size();
		u32 sz = (u32)sizeof(disp);
		pushbuf.resize(pushbuf.size() + sz);
		memcpy(pushbuf.data() + ptr, &disp, sz);

		commands.push_back({ CommandType::DISPLAY, sz, ptr });

		FinishRecording();
	}
	if (nextFrame && (gstate_c.skipDrawReason & SKIPDRAW_SKIPFRAME) == 0 && noDisplayAction) {
		NOTICE_LOG(SYSTEM, "Recording starting on frame...");
		BeginRecording();
	}
}

};
