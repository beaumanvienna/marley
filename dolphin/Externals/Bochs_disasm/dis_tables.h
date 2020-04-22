/////////////////////////////////////////////////////////////////////////
// $Id: dis_tables.h 11878 2013-10-11 20:09:51Z sshwarts $
/////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2005-2012 Stanislav Shwartsman
//          Written by Stanislav Shwartsman [sshwarts at sourceforge net]
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef _BX_DISASM_TABLES_
#define _BX_DISASM_TABLES_

// opcode table attributes
#define _GROUPN        1
#define _SPLIT11B      2
#define _GRPFP         3
#define _GRP3DNOW      4
#define _GRPSSE        5
#define _GRPSSE66      6
#define _GRPSSEF2      7
#define _GRPSSEF3      8
#define _GRPSSENONE    9
#define _GRPSSE2       10
#define _GRPRM         11
#define _GRP3BOP       12
#define _GRP64B        13
#define _GRPVEXW       14

/* ************************************************************************ */
#define GRPSSE(n)       _GRPSSE,   BxDisasmGroupSSE_##n
#define GRPSSE2(n)      _GRPSSE2,  BxDisasmGroupSSE_##n
#define GRPAVX(n)       _GRPSSE,   BxDisasmGroupAVX_##n
#define GRPAVX2(n)      _GRPSSE2,  BxDisasmGroupAVX_##n
#define GRPN(n)         _GROUPN,   BxDisasmGroup##n
#define GRPRM(n)        _GRPRM,    BxDisasmGroupRm##n
#define GRPMOD(n)       _SPLIT11B, BxDisasmGroupMod##n
#define GRPFP(n)        _GRPFP,    BxDisasmFPGroup##n
#define GRP3DNOW        _GRP3DNOW, BxDisasm3DNowGroup
#define GR3BTAB(n)      _GRP3BOP,  BxDisasm3ByteOpTable##n
#define GR64BIT(n)      _GRP64B,   BxDisasmGrpOs64B_##n
#define GRPVEXW(n)      _GRPVEXW,  BxDisasmGrpVexW_##n
/* ************************************************************************ */

/* ************************************************************************ */
#define GRPSSE66(n)     _GRPSSE66,   &n
#define GRPSSEF2(n)     _GRPSSEF2,   &n
#define GRPSSEF3(n)     _GRPSSEF3,   &n
#define GRPSSENONE(n)   _GRPSSENONE, &n
/* ************************************************************************ */

#define Apw &DO_disassembler::Apw
#define Apd &DO_disassembler::Apd

#define AL_Reg &DO_disassembler::AL_Reg
#define CL_Reg &DO_disassembler::CL_Reg
#define AX_Reg &DO_disassembler::AX_Reg
#define DX_Reg &DO_disassembler::DX_Reg

#define EAX_Reg &DO_disassembler::EAX_Reg
#define RAX_Reg &DO_disassembler::RAX_Reg
#define RCX_Reg &DO_disassembler::RCX_Reg

#define CS &DO_disassembler::CS
#define DS &DO_disassembler::DS
#define ES &DO_disassembler::ES
#define SS &DO_disassembler::SS
#define FS &DO_disassembler::FS
#define GS &DO_disassembler::GS

#define Sw &DO_disassembler::Sw

#define Cd &DO_disassembler::Cd
#define Cq &DO_disassembler::Cq

#define Dd &DO_disassembler::Dd
#define Dq &DO_disassembler::Dq

#define Reg8 &DO_disassembler::Reg8
#define   RX &DO_disassembler::RX
#define  ERX &DO_disassembler::ERX
#define  RRX &DO_disassembler::RRX

#define Eb  &DO_disassembler::Eb
#define Ew  &DO_disassembler::Ew
#define Ed  &DO_disassembler::Ed
#define Eq  &DO_disassembler::Eq
#define Ey  &DO_disassembler::Ey
#define Ebd &DO_disassembler::Ebd
#define Ewd &DO_disassembler::Ewd

#define Gb &DO_disassembler::Gb
#define Gw &DO_disassembler::Gw
#define Gd &DO_disassembler::Gd
#define Gq &DO_disassembler::Gq
#define Gy &DO_disassembler::Gy

#define By &DO_disassembler::By

#define I1 &DO_disassembler::I1
#define Ib &DO_disassembler::Ib
#define Iw &DO_disassembler::Iw
#define Id &DO_disassembler::Id
#define Iq &DO_disassembler::Iq

#define IbIb &DO_disassembler::IbIb
#define IwIb &DO_disassembler::IwIb

#define sIbw &DO_disassembler::sIbw
#define sIbd &DO_disassembler::sIbd
#define sIbq &DO_disassembler::sIbq
#define sIdq &DO_disassembler::sIdq

#define ST0 &DO_disassembler::ST0
#define STi &DO_disassembler::STi

#define Rw &DO_disassembler::Rw
#define Rd &DO_disassembler::Rd
#define Rq &DO_disassembler::Rq
#define Ry &DO_disassembler::Ry

#define Pq &DO_disassembler::Pq
#define Qd &DO_disassembler::Qd
#define Qq &DO_disassembler::Qq
#define Nq &DO_disassembler::Nq

#define  Vq &DO_disassembler::Vq
#define Vdq &DO_disassembler::Vdq
#define Vss &DO_disassembler::Vss
#define Vsd &DO_disassembler::Vsd
#define Vps &DO_disassembler::Vps
#define Vpd &DO_disassembler::Vpd
#define VIb &DO_disassembler::VIb

#define Ups &DO_disassembler::Ups
#define Upd &DO_disassembler::Upd
#define Udq &DO_disassembler::Udq
#define Uq  &DO_disassembler::Uq

#define  Wb &DO_disassembler::Wb
#define  Ww &DO_disassembler::Ww
#define  Wd &DO_disassembler::Wd
#define  Wq &DO_disassembler::Wq
#define Wdq &DO_disassembler::Wdq
#define Wss &DO_disassembler::Wss
#define Wsd &DO_disassembler::Wsd
#define Wps &DO_disassembler::Wps
#define Wpd &DO_disassembler::Wpd

#define Hdq &DO_disassembler::Hdq
#define Hps &DO_disassembler::Hps
#define Hpd &DO_disassembler::Hpd
#define Hss &DO_disassembler::Hss
#define Hsd &DO_disassembler::Hsd

#define Ob &DO_disassembler::Ob
#define Ow &DO_disassembler::Ow
#define Od &DO_disassembler::Od
#define Oq &DO_disassembler::Oq

#define  Ma &DO_disassembler::Ma
#define  Mp &DO_disassembler::Mp
#define  Ms &DO_disassembler::Ms
#define  Mx &DO_disassembler::Mx
#define  Mb &DO_disassembler::Mb
#define  Mw &DO_disassembler::Mw
#define  Md &DO_disassembler::Md
#define  Mq &DO_disassembler::Mq
#define  Mt &DO_disassembler::Mt
#define Mdq &DO_disassembler::Mdq
#define Mps &DO_disassembler::Mps
#define Mpd &DO_disassembler::Mpd
#define Mss &DO_disassembler::Mss
#define Msd &DO_disassembler::Msd

#define VSib &DO_disassembler::VSib

#define Xb &DO_disassembler::Xb
#define Xw &DO_disassembler::Xw
#define Xd &DO_disassembler::Xd
#define Xq &DO_disassembler::Xq

#define Yb &DO_disassembler::Yb
#define Yw &DO_disassembler::Yw
#define Yd &DO_disassembler::Yd
#define Yq &DO_disassembler::Yq

#define sYq  &DO_disassembler::sYq
#define sYdq &DO_disassembler::sYdq

#define Jb &DO_disassembler::Jb
#define Jw &DO_disassembler::Jw
#define Jd &DO_disassembler::Jd

#define XX 0

const struct BxDisasmOpcodeInfo_t
#include "opcodes.inc"
#include "dis_tables_x87.inc"
#include "dis_tables_sse.inc"
#include "dis_tables_avx.inc"
#include "dis_tables_xop.inc"
#include "dis_tables.inc"

#undef XX

#endif
