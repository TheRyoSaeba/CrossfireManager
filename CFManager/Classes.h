#pragma once

#pragma warning(push)
#pragma warning(disable: VCR001)

#ifdef __INTELLISENSE__
#define DEFINE_MEMBER_0(t, n) t n
#define DEFINE_MEMBER_N(o, t, n) t n
#else
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_0(t, n) struct { t n; }
#define DEFINE_MEMBER_N(o, t, n) struct { unsigned char MAKE_PAD(o); t n; }
#endif

#pragma warning(pop)

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
#include <map>
#include "../Memory/Memory.h"
#include "Shellcode.h"
#include "offsets.h"

using namespace DirectX;
namespace KLASSES {

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
    class CWeapon;
    class CCharacterHitBox;

    class pPlayer
    {
    public:
        union
        {
            DEFINE_MEMBER_0(obj*, hObject);
            DEFINE_MEMBER_N(0x0008, int8_t, ClientID);
            DEFINE_MEMBER_N(0x0009, int8_t, Team);
            DEFINE_MEMBER_N(0x000A, char, Name[14]);
            DEFINE_MEMBER_N(0x0018, pCharacterFx*, characFX);
            DEFINE_MEMBER_N(0x0020, int32_t, ScoreBoard);
            DEFINE_MEMBER_N(0x0024, int32_t, BOTorNOT);
            DEFINE_MEMBER_N(0x0028, int32_t, C4);
            DEFINE_MEMBER_N(0x002C, int32_t, Host);
            DEFINE_MEMBER_N(0x0030, int64_t, Rank);
            DEFINE_MEMBER_N(0x0038, int64_t, IDNumber);
            DEFINE_MEMBER_N(0x0040, int32_t, Spectator);
            DEFINE_MEMBER_N(0x0044, int32_t, Health);
            DEFINE_MEMBER_N(0x0048, int8_t, Kills);
            DEFINE_MEMBER_N(0x0049, int8_t, Deaths);
            DEFINE_MEMBER_N(0x004C, int8_t, ModelType);
            DEFINE_MEMBER_N(0x004D, char, pad_004D[3219]);
            DEFINE_MEMBER_N(0x0CE0, int16_t, ACE);
            DEFINE_MEMBER_N(0x0CE2, char, pad_0CE2[86]);
            DEFINE_MEMBER_N(0x0D38, char, padding[0x90]);
        };
    };

    class CCharacterHitBox
    {
        public:
    
          static  bool EnlargeDims(Memory& m, CCharacterHitBox* h, float s) {
                if (!h) return false;

                uintptr_t f = CFSHELL + 0x15C44D0;

                std::vector<uint8_t> b = {
                    0x48, 0x83, 0xEC, 0x28,
                    0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0,
                    0xF3, 0x0F, 0x10, 0x05, 0x24, 0x00, 0x00, 0x00,
                    0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,
                    0xFF, 0xD0,
                    0x48, 0x83, 0xC4, 0x28,
                    0xC3,
                    0, 0, 0, 0
                };

                memcpy(&b[6], &h, sizeof(h));
                memcpy(&b[26], &f, sizeof(f));
                memcpy(&b[b.size() - 4], &s, sizeof(s));

                uint64_t v = m.shellcode.find_codecave(b.size(), "crossfire.exe", "crossfire.exe");
                if (!v) return false;

                if (!m.Write(v, b.data(), b.size())) return false;

                uintptr_t t = m.GetImportTableAddress("timeGetTime", "crossfire.exe", "WINMM.dll");
                if (!t) return false;

                return m.shellcode.call_function((void*)t, (void*)v, "crossfire.exe");
            }

    };


    class pCharacterFx
    {
    public:
        union
        {
            DEFINE_MEMBER_0(void*, ObjectFX);
            DEFINE_MEMBER_N(0x06D0, int8_t, isDead);
            DEFINE_MEMBER_N(0x0708, CCharacterHitBox*, pCharacterHitBox);
            DEFINE_MEMBER_N(0x11518, CWeapon*, WeaponOnHand);
        };
    };

     
    

    class CWeapon
    {
    public:
        union {
            DEFINE_MEMBER_0(int16_t, WeaponID);
            DEFINE_MEMBER_N(0x0002, int16_t, WeaponClass);
            DEFINE_MEMBER_N(0x0010, char, WeaponName[28]);
            DEFINE_MEMBER_N(0x0170, float, Range);
            DEFINE_MEMBER_N(0x04AC, float, KnifeRate1);
            DEFINE_MEMBER_N(0x04DC, float, knifebigrate1);
            DEFINE_MEMBER_N(0x050C, float, KnifeRate2);
            DEFINE_MEMBER_N(0x053C, float, knifebigrate2);
            DEFINE_MEMBER_N(0x08FC, float, ReloadRatio);
            DEFINE_MEMBER_N(0x0900, float, AnimRatio);
            DEFINE_MEMBER_N(0x0974, float, DetailPeturbShot2);
            DEFINE_MEMBER_N(0x09B0, float, DetailReactPitchShot1);
            DEFINE_MEMBER_N(0x09C4, float, DetailReactPitchShot2);
            DEFINE_MEMBER_N(0x09EC, float, DetailReactYawShot1);
            
        };
        static inline int16_t GetWeaponIndex(Memory& mem, pPlayer _localPlayer)
        {

            pCharacterFx* pLocalFX = _localPlayer.characFX;
            if (!pLocalFX)
                return -1;
            CWeapon* pCurrentWeapon = mem.Read<CWeapon*>(reinterpret_cast<uintptr_t>(pLocalFX) + offsetof(pCharacterFx, WeaponOnHand));
            if (!pCurrentWeapon)
                return -1;
            int16_t WeaponID = mem.Read<int16_t>(reinterpret_cast<uintptr_t>(pCurrentWeapon) + offsetof(CWeapon, WeaponID));

            return WeaponID;
        }
        static inline WeaponType GetCurrentWeaponType(const pPlayer& localPlayer)
        {
            if (!localPlayer.characFX)
                return WeaponType::Knife;

            VMMDLL_SCATTER_HANDLE scatter = mem.CreateScatterHandle();

            CWeapon* currentWeapon = nullptr;
            mem.AddScatterReadRequest(scatter,
                reinterpret_cast<uintptr_t>(localPlayer.characFX) + offsetof(pCharacterFx, WeaponOnHand),
                &currentWeapon,
                sizeof(currentWeapon));

            mem.ExecuteReadScatter(scatter);

            if (!currentWeapon) {
                mem.CloseScatterHandle(scatter);
                return WeaponType::Knife;
            }

            int16_t weaponClass = 0;
            mem.AddScatterReadRequest(scatter,
                reinterpret_cast<uintptr_t>(currentWeapon) + offsetof(CWeapon, WeaponClass),
                &weaponClass,
                sizeof(weaponClass));

            mem.ExecuteReadScatter(scatter);
            mem.CloseScatterHandle(scatter);

            return static_cast<WeaponType>(weaponClass);
        }
    };

    class LT_DRAWPRIM {
    public:
        char pad_0000[11376]; // 0x0000
        D3DXMATRIX view;      // 0x2C70  <--  
        D3DXMATRIX projection; // 0x2CB0 <--  
        D3DVIEWPORT9 viewport; // 0x2CF0 <--  
    };

    class pObject
    {
    public:
        union
        {
            DEFINE_MEMBER_0(char, pad_0008[1080]);
        };
    };

    class Node {
    public:
        union {
            DEFINE_MEMBER_N(0x004, char, name[0x20]);
            DEFINE_MEMBER_N(0x0038, D3DXVECTOR3, dimension);
        };
        uint8_t padding[0xA4 - 0x3C];

        Node() {
            std::memset(this, 0, 0x0A4);
        }
    };


    class pCamera
    {
    public:
        union
        {
             
            DEFINE_MEMBER_N(0x0098, int32_t, Perspective);
            DEFINE_MEMBER_N(0x00B0, D3DXVECTOR3, camerapos);

        };

    };


    class LTClientShell
    {
    public:
        union
        {
            
            DEFINE_MEMBER_N(0x0040, pCamera*, CCamera);
            DEFINE_MEMBER_N(0x0088, void*, CWorldPropsClnt);
            DEFINE_MEMBER_N(0x0090, pPlayerClntBase*, CPlayerClntBase);
            DEFINE_MEMBER_N(0x0098, int32_t, ingame);
            DEFINE_MEMBER_N(0x00B0, void*, CLTClient);
            DEFINE_MEMBER_N(0x0110, void*, CameraInstance);
            DEFINE_MEMBER_N(0x02B0, void*, CGameUI);
            DEFINE_MEMBER_N(0x02A0, char, NickName[16]);
            DEFINE_MEMBER_N(0x02DC, int32_t, Health);
            DEFINE_MEMBER_N(0x02E0, void*, EntityStart);
        };

        pPlayer GetPlayerByIndex(int index)
        {
            uintptr_t ENTITY_BASE = offs::LT_SHELL + offs::dwCPlayerStart;
            uintptr_t playerAddress = ENTITY_BASE + (index * offs::dwCPlayerSize);
            pPlayer player = mem.Read<pPlayer>(playerAddress);
            return player;
        }

        pPlayer GetLocalPlayer(Memory& memRef)
        {
            int idx = memRef.Read<int>(offs::LT_SHELL + offs::MYOFFSET);
            return GetPlayerByIndex(idx);
        }

        bool inGame()
        {
            return (this->ingame != 0);
        }

        struct Vector2 {
            float x, y;
        };

        struct Transform
        {
            D3DXVECTOR3 Pos;
        };

         

    };

    class pBoneArray
    {
    public:
        D3DXMATRIX BoneMatrix[100];
    };

    class obj
    {
    public:
        union
        {
            DEFINE_MEMBER_N(0x0008, D3DXVECTOR3, foot);
            DEFINE_MEMBER_N(0x0014, D3DXVECTOR3, Head);
            DEFINE_MEMBER_N(0x0020, char, pad_0020[404]);
            DEFINE_MEMBER_N(0x01B4, D3DXVECTOR3, AbsolutePosition);
            DEFINE_MEMBER_N(0x01C0, char, pad_01C0[10096]);
            DEFINE_MEMBER_N(0x2930, pBoneArray*, BoneArray);
            DEFINE_MEMBER_N(0x2938, char, pad_2938[800]);
        };

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

    class CWorldPropsClnt {
    public:
        char pad_0000[2112];
    };

    

    class pGameUI
    {
    public:
        union
        {
            DEFINE_MEMBER_N(0x0018, void*, CSetWeaponUI);
            DEFINE_MEMBER_N(0x0020, char, pad_0020[295]);
        };
    };

    class pBag
    {
    public:
        union
        {
            DEFINE_MEMBER_0(char, pad_0000[12]);
            DEFINE_MEMBER_N(0x000C, int32_t, ToggleBag);
            DEFINE_MEMBER_N(0x0010, char, pad_0010[51]);
        };
    };

    class pPlayerClntBase
    {
    public:
        union
        {
            DEFINE_MEMBER_0(char, pad_0000[992]);
            DEFINE_MEMBER_N(0x03E0, int32_t, Ammo);
            DEFINE_MEMBER_N(0x03E4, int32_t, MaxAmmo);
            DEFINE_MEMBER_N(0x03E8, char, pad_03E8[24]);
            DEFINE_MEMBER_N(0x0400, void*, PlayerViewManager);
            DEFINE_MEMBER_N(0x0408, char, pad_0408[568]);
            DEFINE_MEMBER_N(0x0640, float, Yaw);
            DEFINE_MEMBER_N(0x0644, char, pad_0644[4]);
            DEFINE_MEMBER_N(0x0648, float, Pitch);
            DEFINE_MEMBER_N(0x064C, float, Roll);
            DEFINE_MEMBER_N(0x0650, char, pad_0650[1960]);
            DEFINE_MEMBER_N(0x0DF8, D3DXVECTOR2, ViewAngles);
            DEFINE_MEMBER_N(0x0E00, char, pad_0E00[1600]);
        };
    };

    class PlayerViewManager
    {
    public:
        union
        {
            DEFINE_MEMBER_N(0x00C0, int32_t, N00000692);
            DEFINE_MEMBER_N(0x00C4, int32_t, CurrentWeapon);
            DEFINE_MEMBER_N(0x00F0, float, ZoomLevel);
        };
        static inline float GetZoomLevel(Memory& mem, LTClientShell _shell)
        {
            if (!_shell.CPlayerClntBase)
                return -1;

            VMMDLL_SCATTER_HANDLE scatter = mem.CreateScatterHandle();

            PlayerViewManager* playerViewManager = nullptr;
            mem.AddScatterReadRequest(scatter,
                reinterpret_cast<uintptr_t>(_shell.CPlayerClntBase) + offsetof(pPlayerClntBase, PlayerViewManager),
                &playerViewManager,
                sizeof(playerViewManager));

            mem.ExecuteReadScatter(scatter);

            if (!playerViewManager) {
                mem.CloseScatterHandle(scatter);
                return -1;
            }

            float zoomLevel = 0;
            mem.AddScatterReadRequest(scatter,
                reinterpret_cast<uintptr_t>(playerViewManager) + offsetof(PlayerViewManager, ZoomLevel),
                &zoomLevel,
                sizeof(zoomLevel));

            mem.ExecuteReadScatter(scatter);
            mem.CloseScatterHandle(scatter);

            return zoomLevel;
        }


    };

    inline bool EngineW2S(const LT_DRAWPRIM& drawPrim, D3DXVECTOR3* InOut) {
        D3DXVECTOR3 vScreen;
        D3DXVECTOR3 worldPos = *InOut;

        XMVECTOR worldPosVec = XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);

        XMMATRIX projMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&drawPrim.projection));
        XMMATRIX viewMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&drawPrim.view));
        XMMATRIX worldMatrix = XMMatrixIdentity();

        XMVECTOR screenVec = XMVector3Project(
            worldPosVec,
            0.0f, 0.0f,
            static_cast<float>(drawPrim.viewport.Width), static_cast<float>(drawPrim.viewport.Height),
            0.0f, 1.0f,
            projMatrix,
            viewMatrix,
            worldMatrix
        );

        InOut->x = XMVectorGetX(screenVec);
        InOut->y = XMVectorGetY(screenVec);
        InOut->z = XMVectorGetZ(screenVec);

        return InOut->z <= 1.0f;
    }
}