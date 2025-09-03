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

//USE RVA  [ <CShell_x64.dll> + XXXXXX]

// ModelNode → DamageFactor
#define NODE_OFFSET     0x3191798    //mov rdi, cs:qword_7FFDC981F978
#define NODE_SIZE       0xA4        // 00007FFDC6937D5B: add r14, 0A4h

// ===============================
//       BUG DAMAGE PATCH
// ===============================

#define BUG_DAMAGE      0x2FC       // NoBugDamage offset inside CZoneMan
                                    
#define ZONE_MGR        0x344EFD8  //
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


#define WEAPON_MGR      0x339EC50  // WeaponIndex: mov rcx, qword ptr cs:xmmword_7FFDC9A2CE28



inline uintptr_t CFBASE = 0;
inline uintptr_t CFSHELL = 0;
constexpr int MAX_PLAYERS = 24;
constexpr size_t ENTITY_START = 0x10;
const float FOV_DEFAULT = 2.094f;
 
namespace offs {
    //============================================================
   // Signature Patterns (Fixed)
   //============================================================
   /*
   * 
   * 
   * 
   * 
   * 
       LT_PATTERN:
       ---------------------------------------------------------------------
       :00007FF855806945                 lea     r8, aIclientshellDe ; "IClientShell.Default"
________:00007FF85580694C                 lea     rdx, off_7FF8589AE2A0 <----
________:00007FF855806953                 call    qword ptr [rax+20h]
        ---------------------------------------------------------------------
    */
     
    constexpr auto LT_PATTERN =

        " ";


     
    constexpr auto DRAWPRIM_PATTERN =
        " ";


    /*
        DRAWPRIM_PATTERN:
        ---------------------------------------------------------------------
        00000001400195A9                 lea     r8, aIltdrawprimDef ; "ILTDrawPrim.Default"
        ________:00000001400195B0         lea     rdx, off_140FB6110     <----
        ---------------------------------------------------------------------
    */


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


    

    if (!CFSHELL || !cshell_size || !CFBASE || !cbase_size) {
        LOG_ERROR("Invalid module bases");
        return false;
    }

    const auto scan_in_range = [&](const char* name, const auto& pattern, uintptr_t base, size_t size) {
       // LogScan(name, base, size);
        auto result = mem.FindSignature(pattern, base, base + size);
        return result;
        };

    const auto first = scan_in_range("LT_PATTERN", offs::LT_PATTERN, CFSHELL, cshell_size);
    const auto fourth = scan_in_range("DRAWPRIM_PATTERN", offs::DRAWPRIM_PATTERN, CFBASE, cbase_size);

    if (!first || !fourth ) {
        LOG_ERROR("Signature scan failed. Missing patterns: Restart G");
        if (!first) LOG_INFO2("- LT_PATTERN");
        if (!fourth) LOG_INFO2("- DRAWPRIM_PATTERN");
        return false;
    }
    const auto read_offset = [&](const char* name, auto addr) {
        auto val = mem.Read<int32_t>(addr + 3);
       // LogHex(name, val);
        return val;
        };

    offs::LT_SHELL = first + 7 + read_offset("LT_OFFSET", first);
    offs::ILTDrawPrim = fourth + 7 + read_offset("DRAWPRIM_OFFSET", fourth);
    LOG_INFO("Final offsets:");
  //  LogHex("LT_SHELL", offs::LT_SHELL);
  //  LogHex("ILTDrawPrim", offs::ILTDrawPrim);
 //   LogHex("ENTITY_START", ENTITY_START);
    if (offs::LT_SHELL > (CFSHELL + cshell_size)) {
        LOG_ERROR("Invalid LT_SHELL outside module range");
        return false;
    }
    
    LOG_INFO("[X] Cheats not working? Restart your PC.\n");
    
    return true;
}





