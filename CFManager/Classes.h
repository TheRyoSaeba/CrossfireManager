#pragma once

// ---------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)

#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]

// This macro attempts to put a pad of 'offset' bytes, then place `type name;`
#define DEFINE_OFFSET_FIELD(offset, type, name)  \
  struct {                                       \
    unsigned char MAKE_PAD(__COUNTER__)[offset]; \
    type name;                                   \
  }

#define DEFINE_OFFSET_FIELD_SAFE(offset, type, name)                   \
  struct {                                                             \
    unsigned char MAKE_PAD(__COUNTER__)[(offset) == 0 ? 0 : (offset)]; \
    type name;                                                         \
  }

// ---------------------------------------------------------------------------
// Classes.h was very annoying with only brief respites
// ---------------------------------------------------------------------------
typedef char Byte3219[3219];
typedef char Byte86[86];
typedef char Byte14[14];

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#pragma comment(lib, "d3dx9.lib")
 
#include <DirectXMath.h>

 
#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
 
#include "Memory/Memory.h"
#include "Memory/Shellcode.h"
#include "Shellcode.h"
#include "offsets.h"

#include <map>
 
 

namespace KLASSES {
    
    inline std::atomic<bool> ESPThreadRunning = false;
    inline std::thread ESPThread;
    inline bool no_spread = false;

    namespace Utils {
        inline void DebugLog(const char* format, ...) {
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            OutputDebugStringA(buffer);
            OutputDebugStringA("\n");
        }
    }  // namespace Utils
    /*
    ================================================================================
    SECTION 3: Basic Structs & Vectors
    ================================================================================

    */

    class pPlayer;
    class pCamera;
    class pGameUI;
    class obj;
    class pPlayerClntBase;
    class CWorldPropsClnt;
    class LTClientShell;
    class pBag;
    class PlayerViewManager;
    class pCharacterFx;
    class ILTClient;
    class pObject;

    /******************************************************************************
     *                       PlayerClass                        *
     ******************************************************************************/
    class pPlayer {
    public:
        obj* hObject;            // 0x0000
        int8_t ClientID;         // 0x0008
        int8_t Team;             // 0x0009
        char Name[14];           // 0x000A
        pCharacterFx* characFX;  // 0x0018
        int32_t ScoreBoard;      // 0x0020
        int32_t BOTorNOT;        // 0x0024
        int32_t C4;              // 0x0028
        int32_t Host;            // 0x002C
        int64_t Rank;            // 0x0030
        int64_t IDNumber;        // 0x0038
        int32_t Spectator;       // 0x0040
        int32_t Health;          // 0x0044
        int8_t Kills;            // 0x0048
        int8_t Deaths;           // 0x0049
        char pad_004A[2];        // 0x004A
        int8_t ModelType;        // 0x004C
        char pad_004D[3219];     // 0x004D
        int16_t ACE;             // 0x0CE0
        char pad_0CE2[86];       // 0x0CE2
        char padding[0xDC8 - 0x0D38]; // Add padding to match 0xDC8 - size also equals entity offset
    };

   

    class pCharacterFx {
    public:
        pObject* pObjectFx;         // 0x0000
        char pad_0008[1448];     // 0x0008
        int8_t N00000A28;        // 0x05B0
        int8_t N00000BD0;        // 0x05B1
        char pad_05B2[2];        // 0x05B2
        int8_t Respawn;          // 0x05B4
        char pad_05B5[19];       // 0x05B5
        int8_t isDead;           // 0x05C8
        char pad_05C9[55];       // 0x05C9
        void* pCharacterHitbox;  // 0x0600
        char pad_0608[824];      // 0x0608



    };


    class LT_DRAWPRIM {
    public:
        char pad_0000[11376]; // 0x0000
        D3DXMATRIX view;       // 0x2C70
        D3DXMATRIX projection; // 0x2CB0
        D3DVIEWPORT9 viewport; // 0x2CF0
    };


    class pObject
    {
    public:
      
        char pad_0008[1080]; //0x0008
    }; //Size

    /******************************************************************************
     *                    LTClientShell - Main                       *
     ******************************************************************************/

    class LTClientShell {
    public:
        char pad_0000[64];                 // 0x0000
        pCamera* CCamera;                  // 0x0040
        char pad_0048[64];                 // 0x0048
        void* CWorldPropsClnt;             // 0x0088
        pPlayerClntBase* CPlayerClntBase;  // 0x0090  // use for in game check
        char pad_0098[24];                 // 0x0098
        void* CLTClient;                   // 0x00B0
        char pad_00B8[88];                 // 0x00B8
        void* CameraInstance;              // 0x0110
        char pad_0118[336];                // 0x0118
        pGameUI* CGameUI;                  // 0x0268
        char pad_0270[8];                  // 0x0270
        void* CNewLoadingProduc;           // 0x0278
        char pad_0280[24];                 // 0x0280
        void* ModelInstance;               // 0x0298
        char NickName[16];                 // 0x02A0
        void* GameMode3rd;                 // 0x02B0
        char pad_02B8[32];   // 0x02B8   //local index 0x292, took me too long
        int32_t unk;         // 0x02D8
        int32_t Health;      // 0x02DC
        int32_t N00000189;   // 0x02E0
        char pad_02E4[876];  // 0x02E4
        pPlayer GetPlayerByIndex(int index) {
            uintptr_t ENTITY_BASE = LT_SHELL + offs::dwCPlayerStart;
            uintptr_t playerAddress = ENTITY_BASE + (index * offs::dwCPlayerSize);
            pPlayer player = mem.Read<pPlayer>(playerAddress);
            return player;
        }
        pPlayer GetLocalPlayer(Memory& mem) {
            int localIdx = mem.Read<int>((uintptr_t)LT_SHELL + offs::MYOFFSET);

            return GetPlayerByIndex(localIdx);
        }
        

        bool inGame() {
            return (this->CPlayerClntBase != nullptr);
        }

        /*
         *alternative maybe
         *
         * pPlayer GetLocalPlayer(Memory& mem) {
         *     uintptr_t localbase = LT_SHELL + offs::MYOFFSET;
         *     uintptr_t localPlayerAddress = localbase + (index *
         *offs::dwCPlayerSize); return mem.Read<pPlayer>(localPlayerAddress);
         * }
         */

         //=============================================================================
         // Size: 0x065
        void Debuglog(Memory& mem) {
            constexpr int MAX_PLAYERS = 16;

            for (int i = 0; i < MAX_PLAYERS; i++) {
                uintptr_t ENTITY_BASE = LT_SHELL + offs::dwCPlayerStart;
                uintptr_t playerAddress = ENTITY_BASE + (i * offs::dwCPlayerSize);
                pPlayer player = this->GetPlayerByIndex(i);

                if (playerAddress == 0) continue;

                char sanitizedName[16] = { 0 };
                bool hasValidName = false;

                for (int j = 0; j < sizeof(player.Name); j++) {
                    if (isprint(player.Name[j])) {
                        sanitizedName[j] = player.Name[j];
                        hasValidName = true;
                    }
                    else {
                        sanitizedName[j] = ' ';
                    }
                }
                sanitizedName[sizeof(sanitizedName) - 1] = '\0';

                Utils::DebugLog(
                    "[Player %2d] Addr: 0x%llX | Name: %-15s | Health: %3d | K/D: "
                    "%2d/%2d",
                    i, playerAddress, sanitizedName, player.Health, player.Kills,
                    player.Deaths);
            }
        }
    };

    ////////////////////////

    struct Vector2 {
        float x, y;
    };

    struct Transform
    {
        D3DXVECTOR3 Pos;
        unsigned char spacer00[0x100];
    };

    /* ----------------------------- pCamera ------------------------------ */
    class pCamera {
    public:
        char pad_0000[100];   // 0x0000
        int32_t Perspective;  // 0x0064
        char pad_0068[927];   // 0x0068

         

    };
    class pBoneArray
    {
    public:
        D3DXMATRIX BoneMatrix[100];

    }; //Size: 0x0068

    /* ----------------------------- obj --------------------------------- */
    // alternate
    /*char pad_0000[8];
    D3DVECTOR3 foot;  // 0x0008
    char pad_0014[4];
    D3DVECTOR3 Head;
    char pad_0020[4];       // 0x0014
    D3DVECTOR3 Position;  // 0x0018
    char pad_002C[253];   // 0x002C*/ //alternate
    // Size: 0x2C67

    //////////////////////
    /*char unk0[4]; //pad
    char pad_0000[8];   // 0x000 → Padding before foot
    D3DXVECTOR3 foot;   // 0x008 → Foot position
    char pad_0014[4];   // 0x00C → Padding before Head
    D3DXVECTOR3 Head; */   // 0x014 → Head position////////////

    // Size: 0x0147

    struct obj {
        char pad_0000[8]; //0x0000
        D3DXVECTOR3 foot; //0x0008
        D3DXVECTOR3 Head; //0x0014
        char pad_0020[404]; //0x0020
        D3DXVECTOR3 AbsolutePosition; //0x01B4
        char pad_01C0[10096]; //0x01C0
        pBoneArray* BoneArray; //0x2930
        char pad_2938[800]; //0x2938



        static D3DXVECTOR3 GetFoot(Memory& mem, uintptr_t hObject) {
            if (!hObject) return { 0.0f, 0.0f, 0.0f };

            uintptr_t footAddr = hObject + offsetof(obj, foot);
            return mem.Read<D3DXVECTOR3>(footAddr);
        }

        static D3DXVECTOR3 GetHead(Memory& mem, uintptr_t hObject) {
            if (!hObject) return { 0.0f, 0.0f, 0.0f };
            uintptr_t headAddr = hObject + offsetof(obj, Head);
            return mem.Read<D3DXVECTOR3>(headAddr);
        }

    };

    /* ----------------------------- CWorldPropsClnt --------------------- */
    class CWorldPropsClnt {
    public:
        char pad_0000[2112];  // 0x0000
    };

    class PlayerViewManager {
    public:
        char pad_0000[192];     // 0x0000
        int32_t N00000692;      // 0x00C0
        int32_t CurrentWeapon;  // 0x00C4
        char pad_00C8[120];     // 0x00C8
    };  // Size: 0x0140

    /* ----------------------------- pGameUI ----------------------------- */

    class pGameUI {
    public:
        char pad_0000[24];         // 0x0000
        class pBag* CSetWeaponUI;  // 0x0018
        char pad_0020[295];        // 0x0020
    };

    /* ----------------------------- pBag -------------------------------- */

    class pBag {
    public:
        char pad_0000[12];
        int32_t ToggleBag;
        char pad_0010[51];

        static void InfiniteBag(Memory& mem) {
            std::thread([&]() {
                while (true) {
                    LTClientShell clientShell = mem.Read<LTClientShell>(LT_SHELL);

                    if (!clientShell.CGameUI) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        continue;
                    }

                    pGameUI gameUI = mem.Read<pGameUI>((uintptr_t)clientShell.CGameUI);
                    if (!gameUI.CSetWeaponUI) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }

                    uintptr_t toggleBagAddr =
                        (uintptr_t)gameUI.CSetWeaponUI + offsetof(pBag, ToggleBag);
                    int unlimited = 0;
                    mem.Write(toggleBagAddr, &unlimited, sizeof(int));

                    Utils::DebugLog("InfiniteBag active at: 0x%llX", toggleBagAddr);
                    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                }
                }).detach();
        }
    };
    /******************************************************************************
     *                   CPlayerClntBase - Main                       *
     ******************************************************************************/

    class pPlayerClntBase {
    public:
        char pad_0000[728];                    // 0x0000
        int32_t Ammo;                          // 0x02D8
        int32_t MaxAmmo;                       // 0x02DC
        char pad_02E0[24];                     // 0x02E0
        PlayerViewManager* PlayerViewManager;  // 0x02F8
        char pad_0300[608];                    // 0x0300
        void* firststModelInstance;            // 0x0560
        void* secondndModelInstance;           // 0x0568
        char pad_0570[8];                      // 0x0570
        float Yaw;                             // 0x0578
        char pad_057C[4];                      // 0x057C
        float Pitch;                           // 0x0580
        char pad_0584[684];                    // 0x0584
        int32_t N0000056A;                     // 0x0830
        char pad_0834[272];                    // 0x0834

        static void nospread(Memory& mem) {
            std::thread([&mem]() {
                while (true) {
                    LTClientShell clientShell = mem.Read<LTClientShell>(LT_SHELL);
                    if (auto playerClntBase = clientShell.CPlayerClntBase) {
                        auto ammoAddr = reinterpret_cast<uintptr_t>(&playerClntBase->Ammo);
                        // int norecoilvalue = 1555;
                        // mem.Write(ammoAddr, &norecoilvalue, sizeof(norecoilvalue));
                        Utils::DebugLog("Ammo is now %d", mem.Read<int>(ammoAddr));

                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                }
                }).detach();
        }
    };

    /* static void nospread(Memory& mem) {
        std::thread([&]() {
            while (true) {
                LTClientShell clientShell = mem.Read<LTClientShell>(LT_SHELL);
                pPlayerClntBase* playerClntBase = clientShell.CPlayerClntBase;

                if (!playerClntBase) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                uintptr_t spreadAddr = (uintptr_t)&playerClntBase->Ammo;

                int norecoilvalue = 1555;

                mem.Write(spreadAddr, &norecoilvalue, sizeof(int));
                int ammoCount = mem.Read<int>(spreadAddr);
                Utils::DebugLog("Ammo is now %d", ammoCount);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            }).detach();
    }
    };*/

    /*
        ================================================================================
        SECTION 6: Helpers & Functions (World2Screen, etc.)
        ================================================================================
    */

     
    using namespace DirectX;

    inline bool EngineW2S(const LT_DRAWPRIM& drawPrim, D3DXVECTOR3* InOut) {
        D3DXVECTOR3 vScreen;
        D3DXVECTOR3 worldPos = *InOut;

        // ✅ Convert D3DXVECTOR3 to DirectXMath XMVECTOR
        XMVECTOR worldPosVec = XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);

        // ✅ Convert D3DXMATRIX to DirectXMath XMMATRIX
        XMMATRIX projMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&drawPrim.projection));
        XMMATRIX viewMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&drawPrim.view));
        XMMATRIX worldMatrix = XMMatrixIdentity();  // D3DXMATRIX(1.0f, 0.0f, 0.0f, ...)

        // ✅ Replace D3DXVec3Project with XMVector3Project
        XMVECTOR screenVec = XMVector3Project(
            worldPosVec,
            0.0f, 0.0f,  // Viewport X, Y
            static_cast<float>(drawPrim.viewport.Width), static_cast<float>(drawPrim.viewport.Height),
            0.0f, 1.0f,  // MinZ, MaxZ
            projMatrix,
            viewMatrix,
            worldMatrix
        );

        // ✅ Convert XMVECTOR back to D3DXVECTOR3
        InOut->x = XMVectorGetX(screenVec);
        InOut->y = XMVectorGetY(screenVec);
        InOut->z = XMVectorGetZ(screenVec);

        return InOut->z <= 1.0f;
    }



    
 
  


    


     
    





    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    |                        Update Offsets                               |
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    inline bool Update(Memory& mem) {
        size_t CSHELL_SIZE = mem.GetBaseSize("CShell_x64.dll");
        if (!CFSHELL || CSHELL_SIZE == 0) return false;
        size_t CFBASE_SIZE = mem.GetBaseSize("crossfire.exe");
        if (!CFBASE || CFBASE_SIZE == 0) return false;
        uintptr_t FirstsigResult =
            mem.FindSignature(offs::LT_PATTERN, CFSHELL, CFSHELL + CSHELL_SIZE);
        if (!FirstsigResult) return false;

        int32_t offset = mem.Read<int32_t>(FirstsigResult + 3);
        LT_SHELL = FirstsigResult + 7 + offset;

        uintptr_t SecondSigResult = mem.FindSignature(offs::MY_OFFSET_PATTERN,
            CFSHELL, CFSHELL + CSHELL_SIZE);
        if (!SecondSigResult) return false;
        offs::MYOFFSET = mem.Read<int32_t>(SecondSigResult + 3);

        Utils::DebugLog("sigResult: 0x%llX, offset: 0x%X, LT_SHELL: 0x%llX",

            FirstsigResult, offset, LT_SHELL);
        Utils::DebugLog("sigResult: 0x%llX, offset: 0x%X, myoffset: 0x%llX",

            SecondSigResult, offset, offs::MYOFFSET);

        uintptr_t ThirdSigResult = mem.FindSignature(offs::MY_PLAYERSIZE_PATTERN,
            CFSHELL, CFSHELL + CSHELL_SIZE);
        if (!ThirdSigResult) return false;


        offs::dwCPlayerSize = mem.Read<int32_t>(ThirdSigResult + 3);
        Utils::DebugLog("sigResult: MY_PLAYERSIZE: 0x%X", offs::dwCPlayerSize);

        uintptr_t FourthSigResult = mem.FindSignature(offs::DRAWPRIM_PATTERN, CFBASE, CFBASE + CFBASE_SIZE);
        if (!FourthSigResult)
            return false;
        int32_t drawprimOffset = mem.Read<int32_t>(FourthSigResult + 3);
        offs::ILTDrawPrim = FourthSigResult + 7 + drawprimOffset;
        Utils::DebugLog("sigResult: 0x%llX, offset: 0x%X, DRAWPRIM: 0x%llX",
            FourthSigResult, drawprimOffset, offs::ILTDrawPrim);


        return  (LT_SHELL <= CFSHELL + CSHELL_SIZE);
    }













     

}  // namespace KLASSES

// ┌─────────────────────────────────────────────────────────────────────────┐
// │  UTILITY FUNCTIONS                                          │
// └─────────────────────────────────────────────────────────────────────────┘

// Example offsets

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|                     UTLITIES                       |
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*static void GlowESP()
{
    uintptr_t glowFunction = CFSHELL + 0xEA1610;  // Correct glow function
address

    // Step 1: Allocate a code cave for the hook
    uint64_t hookAddr = mem.shellcode.find_codecave(32, "crossfire.exe",
"d3dx9_29.dll"); if (!hookAddr)
    {
        KLASSES::Utils::DebugLog("[ERROR] No code cave found.");
        return;
    }

    // Step 2: Hook Payload - Setup RCX (GlowStatus), RDX (EntityID), Call Glow
Function unsigned char hookPayload[22] = { 0xB9, 0x01, 0x00, 0x00, 0x00,  // mov
ecx, 1  (GlowStatus = 1) 0xBA, 0x01, 0x00, 0x00, 0x00,  // mov edx, 1  (EntityID
= 1) 0x48, 0xB8                     // mov rax, [glowFunction]
    };
    *reinterpret_cast<uint64_t*>(hookPayload + 12) = glowFunction;
    hookPayload[20] = 0xFF;  // jmp rax
    hookPayload[21] = 0xE0;

    // Step 3: Write Hook Payload into Code Cave
    mem.Write(hookAddr, hookPayload, sizeof(hookPayload));

    // Step 4: Execute the Glow Function
    mem.shellcode.call_function((void*)hookAddr, nullptr, "crossfire.exe");

    KLASSES::Utils::DebugLog("GlowESP executed at 0x%llX with GlowStatus = 1,
EntityID = 1", glowFunction);
}
*/

// namespace KLASSES
