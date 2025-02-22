#pragma once

#include <cstdint>
#include <d3d9.h>
#include <DirectXMath.h>

//=====================================================================
// Global Variables & Configuration
//=====================================================================
static bool showOffsetFinder = false;
using namespace DirectX;

inline uintptr_t CFBASE = 0;
inline uintptr_t CFSHELL = 0;
inline uintptr_t LT_SHELL = 0;

inline D3DVIEWPORT9 g_view_port;

//=====================================================================
// Offsets and Signature Patterns
//=====================================================================
namespace offs {

    //============================================================
    // Signature Patterns (Fixed)
    //============================================================
    /*
        LT_PATTERN:
        ---------------------------------------------------------------------
        00007FF95FDE3865                 lea     r8, aIclientshellDe ; "IClientShell.Default"
        ________:00007FF95FDE386C         lea     rdx, off_7FF962F25D00
        ---------------------------------------------------------------------
    */
    constexpr auto LT_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 ? 48 8D 0D ? ? ? ? 48 83 C4 ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC 48 8D 0D";

    /*
        DRAWPRIM_PATTERN:
        ---------------------------------------------------------------------
        00000001400195A9                 lea     r8, aIltdrawprimDef ; "ILTDrawPrim.Default"
        ________:00000001400195B0         lea     rdx, off_140FB6110   + 0x2c70
        ---------------------------------------------------------------------
    */
    constexpr auto DRAWPRIM_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 20 48 8D 0D ? ? ? ? 48 83 C4 30 5B E9 ? ? ? ? CC CC CC CC CC 48 8D 0D ? ? ? ?";

    /*
        MY_OFFSET_PATTERN:
        ---------------------------------------------------------------------
        00007FF961845E35                 movzx   eax, byte ptr [rbp+292h]
        ---------------------------------------------------------------------
    */
    constexpr auto MY_OFFSET_PATTERN =
        "0F B6 85 ? ? ? ? 48 69 C8 ? ? ? ? 0F B6 84 29 ? ? ? ? 4C 8B 47";

    /*
        MY_PLAYERSIZE_PATTERN:
        ---------------------------------------------------------------------
        00007FF961845E3C                 imul    rcx, rax, 0DC8h
        ---------------------------------------------------------------------
    */
    constexpr auto MY_PLAYERSIZE_PATTERN =
        "48 69 C8 ? ? ? ? 0F B6 84 29 ? ? ? ? 4C 8B 47";

    /*
        Additional Player Info:
        ---------------------------------------------------------------------
        Player start is the model instance right before this address.
        00007FF961845E43                 movzx   eax, byte ptr [rcx+rbp+2A0h]
        ---------------------------------------------------------------------
    */

    //============================================================
    // Memory Offsets
    //============================================================
    inline uintptr_t MYOFFSET = 0;
    inline uintptr_t dwCPlayerStart = 0x298;
    inline uintptr_t dwCPlayerSize = 0;
    inline uintptr_t ILTDrawPrim = 0;

   
    inline bool ESPDRAWBOX = false;
    inline bool AIMBOTBOX = false;
}
