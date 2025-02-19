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
/*
    ================================================================================
    SECTION 1: Includes & Namespace Declaration
    ================================================================================

*/
#include <DirectXMath.h>
#include <Overlay.h>
 
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

// FTXUI
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/flexbox_config.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ftxui/util/ref.hpp>
#include <map>

template <typename T>
T GetVFuncDMA(uintptr_t vTable, int index, Memory& mem) {
    uintptr_t functionPtr = mem.Read<uintptr_t>(vTable + (index * 8));
    return reinterpret_cast<T>(functionPtr);
}
template <typename Fn>
Fn GetVFuncExternal(Memory& mem, uintptr_t thisPtr, size_t index) {
     
    uintptr_t vtablePtr = 0;
    mem.Read(thisPtr, &vtablePtr, sizeof(vtablePtr));

    // 2) Read the function pointer from [vtablePtr + (index * sizeof(uintptr_t))].
    uintptr_t funcAddr = 0;
    mem.Read(vtablePtr + index * sizeof(uintptr_t), &funcAddr, sizeof(funcAddr));

    // 3) Cast it to your desired function signature.
    return reinterpret_cast<Fn>(funcAddr);
}

using namespace ftxui;
namespace KLASSES {
    /*
     ================================================================================
     SECTION 2: Globals & Utilities
     ================================================================================
    */
    // declared extern if needed in other .cpp
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
    char pad_0000[11376];
    D3DXMATRIX view;
    D3DXMATRIX projection;
    D3DVIEWPORT9 viewport;
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

    struct Transform {
        D3DXVECTOR3 Pos;
        char _UnkSpace[0x100];
    };

    /* ----------------------------- pCamera ------------------------------ */
    class pCamera {
    public:
        char pad_0000[100];   // 0x0000
        int32_t Perspective;  // 0x0064
        char pad_0068[927];   // 0x0068
    };

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
        char pad_0000[8];  // 0x0000
        D3DXVECTOR3 foot;  // 0x0008
        D3DXVECTOR3 Head;  // 0x0014

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

    inline bool World2Screen(D3DXVECTOR3* InOut) {
        D3DXMATRIX g_view_matx = mem.Read<D3DXMATRIX>(offs::ViewMatrix);
        D3DXMATRIX g_view_proj = mem.Read<D3DXMATRIX>(offs::ViewMatrix + 0x40);
        D3DVIEWPORT9 g_view_port = mem.Read<D3DVIEWPORT9>(offs::ViewMatrix + 0x80);
        D3DXMATRIX g_view_wrld =
            D3DXMATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        D3DXVECTOR3 vScreen;
        D3DXVECTOR3 PlayerPos(InOut->x, InOut->y, InOut->z);

        D3DXVec3Project(&vScreen, &PlayerPos, &g_view_port, &g_view_proj,
            &g_view_matx, &g_view_wrld);
        if (vScreen.z <= 1.0f) {
            *InOut = vScreen;
            return true;
        }
        return false;
    }
inline bool EngineW2S(const LT_DRAWPRIM& drawPrim, D3DXVECTOR3* InOut) {
    D3DXVECTOR3 vScreen;
    D3DXVECTOR3 worldPos = *InOut;
    D3DXMATRIX world = D3DXMATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    D3DXVec3Project(&vScreen, &worldPos, &drawPrim.viewport, &drawPrim.projection, &drawPrim.view, &world);
    if (vScreen.z <= 1.0f) { *InOut = vScreen; return true; }
    return false;
}

    class CLTModelServer {
    public:
        using GetNodeTransformFn = uint32_t(__stdcall*)(uintptr_t thisPtr,
            obj* hObject, uint32_t Bone,
            Transform* Trans,
            bool WorldSpace);

    private:
        uintptr_t vTable = 0;
        GetNodeTransformFn aGetNodeTransform = nullptr;
        Memory& mem;  // Reference to DMA Memory reader

    public:
        explicit CLTModelServer(Memory& memory) : mem(memory) {
            vTable = CFBASE + 0X4695B8;  // ✅ Make sure this is correct
            if (vTable) {
                aGetNodeTransform = GetVFuncDMA<GetNodeTransformFn>(
                    vTable, 62, mem);  // ✅ Uses correct VTable index
            }
        }

        bool IsValid() const { return vTable != 0 && aGetNodeTransform != nullptr; }

        bool SafeGetNodeTransform(uintptr_t CLTModelServerPtr, obj* playerModel,
            uint32_t Bone, Transform* OutTrans) {
            if (!CLTModelServerPtr) {
                Utils::DebugLog(
                    "❌ CLTModelServerPtr is NULL! Cannot execute GetNodeTransform.");
                return false;
            }

            if (!aGetNodeTransform) {
                Utils::DebugLog("❌ aGetNodeTransform function pointer is NULL!");
                return false;
            }

            if (!playerModel) {
                Utils::DebugLog("❌ playerModel is NULL! Cannot get node transform.");
                return false;
            }

            return aGetNodeTransform(CLTModelServerPtr, playerModel, Bone, OutTrans,
                true);
        }

        void GetNodePosition(obj* playerModel, uint32_t Bone, D3DXVECTOR3& Out) {
            if (!playerModel) {
                Utils::DebugLog("❌ GetNodePosition failed! playerModel is NULL.");
                return;
            }

            if (!aGetNodeTransform) {
                Utils::DebugLog("❌ aGetNodeTransform is NULL! Cannot execute function.");
                return;
            }

            uintptr_t CLTModelServerPtr = mem.Read<uintptr_t>(vTable);
            if (!CLTModelServerPtr) {
                Utils::DebugLog("❌ Failed to read CLTModelServer instance!");
                return;
            }

            Transform Trans;
            Utils::DebugLog("🔍 Calling GetNodeTransform at 0x%llX",
                (uintptr_t)aGetNodeTransform);

            if (SafeGetNodeTransform(CLTModelServerPtr, playerModel, Bone, &Trans)) {
                Out = Trans.Pos;
                Out.y += 5;
            }
            else {
                Utils::DebugLog("❌ GetNodeTransform execution failed!");
            }
        }

        D3DXVECTOR3 GetBonePosition(obj* playerModel, uint32_t Bone) {
            D3DXVECTOR3 out{};
            GetNodePosition(playerModel, Bone, out);
            return out;
        }
    };
    class ILTClient {
    public:
        using SetObjectOutLineFn = void(__stdcall*)(uintptr_t thisPtr,
            uintptr_t Object, bool Status,
            BYTE R, BYTE G, BYTE B);

    private:
        uintptr_t vTable = 0;
        SetObjectOutLineFn aSetObjectOutLine = nullptr;
        Memory& mem;  // DMA Memory Reference

    public:
        explicit ILTClient(Memory& memory) : mem(memory) {
            vTable = CFBASE + 0x457960;  // ✅ Make sure this address is correct
            if (vTable) {
                aSetObjectOutLine = GetVFuncDMA<SetObjectOutLineFn>(
                    vTable, 82, mem);  // ✅ Uses correct VTable index (33)
            }
        }

        bool IsValid() const { return vTable != 0 && aSetObjectOutLine != nullptr; }

        void SetGlow(pCharacterFx* targetObject, bool enable, BYTE R, BYTE G,
            BYTE B) {
            if (!targetObject) {
                Utils::DebugLog("❌ SetGlow failed! Target object is NULL.");
                return;
            }

            if (!aSetObjectOutLine) {
                Utils::DebugLog("❌ aSetObjectOutLine function pointer is NULL!");
                return;
            }

            uintptr_t ILTClientPtr = mem.Read<uintptr_t>(vTable);
            if (!ILTClientPtr) {
                Utils::DebugLog("❌ Failed to read ILTClient instance!");
                return;
            }

            Utils::DebugLog("✨ Calling SetObjectOutLine at 0x%llX",
                (uintptr_t)aSetObjectOutLine);

            aSetObjectOutLine(ILTClientPtr, (uintptr_t)targetObject, enable, R, G, B);
        }

        void TestGlowESP(Memory& mem, pCharacterFx* targetObject) {
            ILTClient ltClient(mem);

            if (!ltClient.IsValid()) {
                Utils::DebugLog("❌ ILTClient is invalid! GlowESP cannot run.");
                return;
            }

            if (!targetObject) {
                Utils::DebugLog("❌ Target object is NULL!");
                return;
            }

            Utils::DebugLog("🛑 Applying Glow ESP to hObject: 0x%llX",
                (uintptr_t)targetObject);

            // Apply Red Glow
            ltClient.SetGlow(targetObject, true, 255, 0, 0);
        }
    };

   

    inline void HookGlowESP(Memory& mem, pObject* pObjectFx, bool enable, BYTE R, BYTE G, BYTE B) {
        if (!pObjectFx) {
            Utils::DebugLog("❌ pObjectFx is NULL! Cannot apply Glow ESP.");
            return;
        }
        
        // ✅ Step 1: Locate a Code Cave in `d3dx9_29.dll`
        uintptr_t codeCave = mem.shellcode.find_codecave(1024, "crossfire.exe", "d3dx9_29.dll");
        if (!codeCave) {
            Utils::DebugLog("❌ No executable memory found in d3dx9_29.dll!");
            return;
        }

        // ✅ Step 2: Prepare the Hook Shellcode
        BYTE shellcode[] = {
            0x48, 0xB8, 0xEF, 0xBE, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00, // mov rax, FunctionAddress
            0xFF, 0xE0 // jmp rax
        };
        *(uintptr_t*)&shellcode[2] = 0x00000001400501E0;  // ✅ Address of SetObjectOutLine

        // ✅ Step 3: Inject the Shellcode Hook
        void* hookAddress = (void*)codeCave;
        mem.shellcode.call_function(hookAddress, (void*)shellcode, "crossfire.exe");

        // ✅ Step 4: Execute the Hooked Function
        struct GlowParams {
            uintptr_t thisPtr;
            uintptr_t Object;
            bool Status;
            BYTE R, G, B;
        };

        GlowParams params = {
            (uintptr_t)hookAddress,
            (uintptr_t)pObjectFx,
            enable,
            R, G, B
        };

        mem.shellcode.call_function(hookAddress, &params, "crossfire.exe");

        // ✅ Debug Log
        Utils::DebugLog("✨ Glow Applied -> pObjectFx: 0x%llX, R: %d, G: %d, B: %d",
            (uintptr_t)pObjectFx, R, G, B);
    }

    inline void HookGlowESP(Memory& mem, obj* hObject, bool enable, BYTE R, BYTE G, BYTE B) {
        if (!hObject) {
            Utils::DebugLog("❌ hObject is NULL! Cannot apply Glow ESP.");
            return;
        }

        
        uintptr_t codeCave = mem.shellcode.find_codecave(1024, "crossfire.exe", "d3dx9_29.dll");
        if (!codeCave) {
            Utils::DebugLog("❌ No executable memory found in d3dx9_29.dll!");
            return;
        }

       
        BYTE shellcode[] = {
            0x48, 0xB8,                      // mov rax, <FunctionAddress>
            0xEF, 0xBE, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xE0                       // jmp rax
        };

        
        *(uintptr_t*)&shellcode[2] = 0x00000001400501E0; // ✅ Address of SetObjectOutLine (example)

        
        void* hookAddress = reinterpret_cast<void*>(codeCave);
        mem.shellcode.call_function(hookAddress, reinterpret_cast<void*>(shellcode), "crossfire.exe");

        // ✅ Step 4: Execute the Hooked Function
        struct GlowParams {
            uintptr_t thisPtr;
            uintptr_t targetObj;
            bool status;
            BYTE R, G, B;
        };

        GlowParams params = {
            (uintptr_t)hookAddress,
            reinterpret_cast<uintptr_t>(hObject),
            enable,
            R, G, B
        };

        mem.shellcode.call_function(hookAddress, &params, "crossfire.exe");

        // ✅ Debug Log
        Utils::DebugLog(
            "✨ Glow Applied -> hObject: 0x%llX, R: %d, G: %d, B: %d",
            (uintptr_t)hObject, R, G, B
        );
    }


 


    inline void HookGlowESPS(Memory& mem, uintptr_t hObject, bool enable, BYTE R, BYTE G, BYTE B)
    {
  #pragma pack(push,1)
        struct {
            bool doGlow;
            uintptr_t funcPtr;
            uintptr_t objectPtr;
            uintptr_t targetObj;
            bool en;
            BYTE rr, gg, bb;
        } params{};
  #pragma pack(pop)

        static bool installed = false;
        static uintptr_t codeCave = 0, paramAddr = 0, stubAddr = 0;

        if (!installed)
        {
            auto moduleList = mem.GetModuleList("crossfire.exe");
            for (auto& mod : moduleList) {
                if (mod.find(".dll") != std::string::npos) { // ✅ Only check DLLs
                    codeCave = mem.shellcode.find_codecave(512, "crossfire.exe", mod.c_str());
                    if (codeCave) {
                        Utils::DebugLog("✅ Found Code Cave -> Module: %s -> Address: 0x%llX", mod.c_str(), codeCave);
                        break;
                    }
                }
            }
            if (!codeCave) {
                Utils::DebugLog("❌ No executable memory found in any module!");
                return;
            }
            paramAddr = codeCave;
            stubAddr = codeCave + sizeof(params);

            BYTE stub[] = {
                0x48,0x83,0xEC,0x28,            // sub rsp,28h
                0x48,0xBB,                     // mov rbx, <paramAddr> (qword)
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x80,0x3B,0x00,                // cmp byte ptr [rbx],0
                0x74,0x1F,                     // je +0x1F
                0xC6,0x03,0x00,                // mov byte ptr [rbx],0
                0x48,0x8B,0x4B,0x10,           // mov rcx,[rbx+0x10]
                0x48,0x8B,0x53,0x18,           // mov rdx,[rbx+0x18]
                0x8A,0x4B,0x20,                // mov cl,[rbx+0x20]
                0x44,0x8A,0x43,0x21,           // mov r8b,[rbx+0x21]
                0x44,0x8A,0x4B,0x22,           // mov r9b,[rbx+0x22]
                0x48,0x8B,0x03,                // mov rax,[rbx]
                0x50,                          // push rax
                0x8A,0x5B,0x23,                // mov bl,[rbx+0x23] (B)
                0x53,                          // push rbx (simulate 5th param on stack)
                0x48,0x8B,0x43,0x08,           // mov rax,[rbx+8] (funcPtr)
                0xFF,0xD0,                     // call rax
                0x58,                          // pop rax (clean)
                0x5B,                          // pop rbx
                0x48,0x83,0xC4,0x28,           // add rsp,28h
                0xE9,0x00,0x00,0x00,0x00       // jmp <+???> (no return to original)
            };

            *(uintptr_t*)&stub[6] = paramAddr;
            mem.Write(stubAddr, stub, sizeof(stub));
            
            mem.shellcode.install_hook((void*)0x00007FF95E6C56D0, (void*)stubAddr, "crossfire.exe");

            installed = true;
        }

        params.doGlow = true;
        params.funcPtr = 0x00000001400501E0;
        params.objectPtr = 0;
        params.targetObj = hObject;
        params.en = enable;
        params.rr = R;
        params.gg = G;
        params.bb = B;
        mem.Write(paramAddr, &params, sizeof(params));
    }




    inline void OutlineOnePlayer(Memory& mem, uintptr_t objectPtr, bool enable, BYTE R, BYTE G, BYTE B)
    {
        if (!objectPtr) return;

        // 1) Code cave
        uintptr_t cave = mem.shellcode.find_codecave(128, "crossfire.exe", "d3dx9_29.dll");
        if (!cave) return;

        // 2) Our ephemeral target function to patch. 
        //    It MUST be something the game calls often. Example: 0x140100000 (placeholder).
        void* ephemeralFunc = (void*)0x140100000;

        // 3) Prepare a small stub. We'll do:
        //    sub rsp, 0x28
        //    mov rcx, <objectPtr>
        //    mov dl, <enable>
        //    mov r8b, <R>
        //    mov r9b, <G>
        //    push <B>    (5th param on stack)
        //    mov rax, 0x1400501E0
        //    call rax
        //    add rsp, 0x28
        //    ret
        BYTE stub[] = {
            0x48, 0x83, 0xEC, 0x28,                // sub rsp,0x28
            0x48, 0xB9,                            // mov rcx, <objectPtr> (8 bytes)
            0,0,0,0,0,0,0,0,
            0xB2, 0x00,                            // mov dl, 0x00
            0x41, 0xB0, 0x00,                      // mov r8b, 0x00
            0x41, 0xB1, 0x00,                      // mov r9b, 0x00
            0x68, 0x00, 0x00, 0x00, 0x00,          // push 0x00000000 (placeholder for B)
            0x48, 0xB8,                            // mov rax, <0x1400501E0> (8 bytes)
            0,0,0,0,0,0,0,0,
            0xFF, 0xD0,                            // call rax
            0x48, 0x83, 0xC4, 0x28,                // add rsp,0x28
            0xC3                                   // ret
        };

        // Fill placeholders:
        *(uint64_t*)&stub[6] = (uint64_t)objectPtr;       // RCX
        stub[16] = (enable ? 1 : 0);          // DL
        stub[19] = R;                         // R8b
        stub[22] = G;                         // R9b
        *(uint32_t*)&stub[25] = (uint32_t)B;               // push B
        *(uint64_t*)&stub[31] = 0x00000001400501E0ULL;     // ObjectOutline code

        // 4) Write stub to code cave
        mem.Write(cave, stub, sizeof(stub));

        // 5) Ephemeral hook: temporarily patch ephemeralFunc with jump -> cave
        //    Overwrites ephemeralFunc with a jmp for ~100ms, then reverts
        mem.shellcode.call_function((void*)cave, ephemeralFunc, "crossfire.exe");
    }








    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    |                        Update Offsets                               |
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    inline bool Update(Memory& mem) {
        size_t CSHELL_SIZE = mem.GetBaseSize("CShell_x64.dll");
        if (!CFSHELL || CSHELL_SIZE == 0) return false;

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
        return (LT_SHELL >= CFSHELL && LT_SHELL < CFSHELL + CSHELL_SIZE);
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
