#pragma once

#include <cstdint>
#include <d3d9.h>
#include <DirectXMath.h>
static bool showOffsetFinder = false;
using namespace DirectX;

 
  inline uintptr_t CFBASE = 0;
  inline uintptr_t CFSHELL = 0;
  inline uintptr_t LT_SHELL = 0;

 
  inline  D3DVIEWPORT9 g_view_port;

namespace offs {
    // ==============================
    // Signature Patterns (Fixed)
    // ==============================
    constexpr auto LT_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 ? 48 8D 0D ? ? ? ? 48 83 C4 ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC 48 8D 0D";
    /*
     ================================================================================
    :00000001400195A9                 lea     r8, aIltdrawprimDef ; "ILTDrawPrim.Default"
________:00000001400195B0                 lea     rdx, off_140FB6110   + 0x2c70
     ================================================================================
    */
    // declared extern if needed in other .cpp
    constexpr auto VIEWMATRIX_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 20 48 8D 0D ? ? ? ? 48 83 C4 30 5B E9 ? ? ? ? CC CC CC CC CC 48 8D 0D ? ? ? ?";


   

    constexpr auto MY_OFFSET_PATTERN =
        "0F B6 85 ? ? ? ? 48 69 C8 ? ? ? ? 0F B6 84 29 ? ? ? ? 4C 8B 47";

    constexpr auto MY_PLAYERSIZE_PATTERN =
        "48 69 C8 ? ? ? ? 0F B6 84 29 ? ? ? ? 4C 8B 47";
     
       
    constexpr auto  ViewMatrix = 0x140FB8D80;
    inline uintptr_t MYOFFSET = 0 ;
    inline uintptr_t dwCPlayerStart = 0x298;
    inline uintptr_t dwCPlayerSize = 0 ;
    inline uintptr_t ILTDrawPrim = 0x140FB6110;;
    inline bool ESPDRAWBOX;
}  
 
 
constexpr float PITCH_MAX = 1539.0f;   
constexpr float YAW_MAX = 180.0f;    

inline float ConvertToCrossfireAngles(float angle, bool isYaw) {
    return isYaw ? fmod(angle + 180.0f, 360.0f) - 180.0f
        : (angle / 1.0f) * PITCH_MAX;
}

inline float ConvertFromCrossfireAngles(float crossfireAngle, bool isYaw) {
    return isYaw ? fmod(crossfireAngle + 180.0f, 360.0f) - 180.0f
        : (crossfireAngle / PITCH_MAX) * 180.0f;
}


 