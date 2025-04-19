#pragma once
#pragma once

#include <cstdint>
#include <string>
#include <array>
#include "CFManager.h"
#include "../Config/globals.h"

//lot of credits to MPGH

// ===============================
//         MODEL NODE
// ===============================

// ModelNode → DamageFactor
#define NODE_OFFSET     0x30BF978   // mov rdi, cs:qword_7FFDC981F978
#define NODE_SIZE       0xA4        // 00007FFDC6937D5B: add r14, 0A4h

// ===============================
//       BUG DAMAGE PATCH
// ===============================

#define BUG_DAMAGE      0x2FC       // NoBugDamage offset inside CZoneMan
                                    
#define ZONE_MGR        0x3380678   //
/* Constructor:
                                     call    sub_7FFDC8627FC0
                                       nop
                                       ...
                                       mov cs:qword_7FFDC9AE0678, rax ← CZoneMan */
// ===============================
//     SHOOT THROUGH WALL (STW)
// ===============================

#define TEXTURE_STRUCT_SIZE  0x82C   // TextureType size

#define WALLADDRESS     0x30C0000   // BulletMarkFX base
#define OFFSET_ONE      0x7DC       // EdgeShotEnabled
#define OFFSET_TWO      0x7E0       // WallShotEnabled
#define OFFSET_THREE    0x7E4       // PerfectWall


#define WEAPON_MGR      0x32CCE28   // WeaponIndex: mov rcx, qword ptr cs:xmmword_7FFDC9A2CE28



inline uintptr_t CFBASE = 0;
inline uintptr_t CFSHELL = 0;
constexpr int MAX_PLAYERS = 24;
const float FOV_DEFAULT = 2.094f;
 
namespace offs {
    
    //============================================================
    // Signature Patterns (Fixed)
    //============================================================
    /*
        LT_PATTERN:
        ---------------------------------------------------------------------
        :00007FF855806945                 lea     r8, aIclientshellDe ; "IClientShell.Default"
________:00007FF85580694C                 lea     rdx, off_7FF8589AE2A0 <----
________:00007FF855806953                 call    qword ptr [rax+20h]
        ---------------------------------------------------------------------
    */
    constexpr auto LT_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 ? 48 8D 0D ? ? ? ? 48 83 C4 ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC 48 8D 0D";

    /*
        DRAWPRIM_PATTERN:
        ---------------------------------------------------------------------
        00000001400195A9                 lea     r8, aIltdrawprimDef ; "ILTDrawPrim.Default"
        ________:00000001400195B0         lea     rdx, off_140FB6110     <----
        ---------------------------------------------------------------------
    */
    constexpr auto DRAWPRIM_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 20 48 8D 0D ? ? ? ? 48 83 C4 30 5B E9 ? ? ? ? CC CC CC CC CC 48 8D 0D ? ? ? ?";

    /*
        MY_OFFSET_PATTERN:
        ---------------------------------------------------------------------
        00007FF856409ACF                 movzx   eax, byte ptr [rbp+2DAh] <----
________:00007FF856409AD6                 imul    rcx, rax, 0DD8h
________:00007FF856409ADD                 movzx   eax, byte ptr [rcx+rbp+2E8h]
        ---------------------------------------------------------------------
    */
    constexpr auto MY_OFFSET_PATTERN =
        "0F B6 85 ? ? ? ? 48 69 C8 D8 0D 00 00 0F B6 84 29 ? ? ? ? 0F BE C0 44 3B F8";

    /*
        MY_PLAYERSIZE_PATTERN:
        ---------------------------------------------------------------------
       00007FF856409ACF                 movzx   eax, byte ptr [rbp+2DAh]
________:00007FF856409AD6                 imul    rcx, rax, 0DD8h   <----
________:00007FF856409ADD                 movzx   eax, byte ptr [rcx+rbp+2E8h]
        ---------------------------------------------------------------------
    */
    constexpr auto MY_PLAYERSIZE_PATTERN =
        "48 69 C8 D8 0D 00 00 0F B6 84 29 ? ? ? ? 0F BE C0 44 3B F8";

    /*
        Additional Player Info:
        ---------------------------------------------------------------------
        Player start is the model instance right before this address in Reclass .
        00007FF856409ADD                 movzx   eax, byte ptr [rcx+rbp+2E8h]
        ---------------------------------------------------------------------
    */

    constexpr auto PLAYER_START_PATTERN = "48 8B 88 ? ? ? ? 48 2B 88 ? ? ? ? 48 C1 F9 03 85 C9 74";
    /*

       MY_PLAYER_START_PATTERN:
    00007FF8569AE3DC                 mov     rcx, [rax + 2E0h]  <----
        ________:00007FF8569AE3E3                 sub     rcx, [rax + 2D8h]
        ________ : 00007FF8569AE3EA                 sar     rcx, 3
        ________ : 00007FF8569AE3EE                 test    ecx, ecx
        ________ : 00007FF8569AE3F0                 jz      short loc_7FF8569AE3FB
        ________ : 00007FF8569AE3F2                 test    bl, bl
        ________ : 00007FF8569AE3F4                 jz      short loc_7FF8569AE3FB
        ________ : 00007FF8569AE3F6 xor r8d, r8d
        ________ : 00007FF8569AE3F9                 jmp     short loc_7FF8569AE402
        ________ : 00007FF8569AE3FB; -------------------------------------------------------------------------- -
        ________:00007FF8569AE3FB
        ________ : 00007FF8569AE3FB loc_7FF8569AE3FB : ; CODE XREF : sub_7FF8569AE2C0 + 112↑j
        ________ : 00007FF8569AE3FB; sub_7FF8569AE2C0 + 130↑j ...
        ________:00007FF8569AE3FB                 mov     r8b, 1
        ________ : 00007FF8569AE3FE                 mov     rdx, [rdi + 20h]
        ________ : 00007FF8569AE402
        ________ : 00007FF8569AE402 loc_7FF8569AE402 : ; CODE XREF : sub_7FF8569AE2C0 + 139↑j
        ________ : 00007FF8569AE402                 mov     rcx, [rdx + 38h]
        ________ : 00007FF8569AE406                 mov     rax, [rcx]
        ________ : 00007FF8569AE409                 xorps   xmm2, xmm2
        ________ : 00007FF8569AE40C                 movzx   edx, r8b
        ________ : 00007FF8569AE410                 call    qword ptr[rax + 2E8h]*/

        //============================================================
        // Memory Offsets
        //============================================================
    inline uintptr_t MYOFFSET = 0;
    inline  uintptr_t dwCPlayerStart = 0;
    inline   uintptr_t dwCPlayerSize = 0;
    inline uintptr_t ILTDrawPrim = 0;
    inline uintptr_t LT_SHELL = 0;

}

 
inline bool PopulateOffsets() {
    if (!mem.vHandle) {
        LOG_ERROR2("Invalid Handle");
        return false;
    }

    const auto cshell_size = mem.GetBaseSize(SHELLNAME);
    const auto cbase_size = mem.GetBaseSize(GAME_NAME);


    auto LogHex = [](const char* name, uintptr_t value) {
        char buf[256];
        sprintf_s(buf, "%s: 0x%llX", name, value);
        LOG_INFO2(buf);
        };

    auto LogScan = [](const char* name, uintptr_t base, size_t size) {
        char buf[256];
        sprintf_s(buf, "Scanning %s at 0x%llX-0x%llX", name, base, base + size);
        LOG_INFO2(buf);
        };


    LogHex("CFSHELL base", CFSHELL);
    LogHex("CFSHELL size", cshell_size);
    LogHex("CFBASE base", CFBASE);
    LogHex("CFBASE size", cbase_size);

    if (!CFSHELL || !cshell_size || !CFBASE || !cbase_size) {
        LOG_ERROR("Invalid module bases");
        return false;
    }

    const auto scan_in_range = [&](const char* name, const auto& pattern, uintptr_t base, size_t size) {
        LogScan(name, base, size);
        auto result = mem.FindSignature(pattern, base, base + size);
        LogHex(name, result);
        return result;
        };

    const auto first = scan_in_range("LT_PATTERN", offs::LT_PATTERN, CFSHELL, cshell_size);
    const auto second = scan_in_range("MY_OFFSET_PATTERN", offs::MY_OFFSET_PATTERN, CFSHELL, cshell_size);
    const auto third = scan_in_range("MY_PLAYERSIZE_PATTERN", offs::MY_PLAYERSIZE_PATTERN, CFSHELL, cshell_size);
    const auto fourth = scan_in_range("DRAWPRIM_PATTERN", offs::DRAWPRIM_PATTERN, CFBASE, cbase_size);
    const auto fifth = scan_in_range("PLAYER_START_PATTERN", offs::PLAYER_START_PATTERN, CFSHELL, cshell_size);

    if (!first || !second || !third || !fourth || !fifth) {
        LOG_ERROR("Signature scan failed. Missing patterns:");
        if (!first) LOG_INFO2("- LT_PATTERN");
        if (!second) LOG_INFO2("- MY_OFFSET_PATTERN");
        if (!third) LOG_INFO2("- MY_PLAYERSIZE_PATTERN");
        if (!fourth) LOG_INFO2("- DRAWPRIM_PATTERN");
        if (!fifth) LOG_INFO2("- PLAYER_START_PATTERN");
        return false;
    }

    const auto read_offset = [&](const char* name, auto addr) {
        auto val = mem.Read<int32_t>(addr + 3);
        LogHex(name, val);
        return val;
        };

    offs::LT_SHELL = first + 7 + read_offset("LT_OFFSET", first);
    offs::MYOFFSET = read_offset("MY_OFFSET", second);
    offs::dwCPlayerSize = read_offset("PLAYER_SIZE", third);
    offs::ILTDrawPrim = fourth + 7 + read_offset("DRAWPRIM_OFFSET", fourth);
    offs::dwCPlayerStart = read_offset("PLAYER_START", fifth);
    LOG_INFO("Final offsets:");
    LogHex("LT_SHELL", offs::LT_SHELL);
    LogHex("MYOFFSET", offs::MYOFFSET);
    LogHex("dwCPlayerSize", offs::dwCPlayerSize);
    LogHex("ILTDrawPrim", offs::ILTDrawPrim);
    LogHex("dwCPlayerStart", offs::dwCPlayerStart);
    if (offs::LT_SHELL > (CFSHELL + cshell_size)) {
        LOG_ERROR("Invalid LT_SHELL outside module range");
        return false;
    }
    
    LOG_INFO("[X] Cheats not working? Restart your PC.\n");
    
    return true;
}





