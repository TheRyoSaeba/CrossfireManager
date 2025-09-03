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
#include <limits>
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
          
            DEFINE_MEMBER_N(0x0008, obj*, hObject);
            DEFINE_MEMBER_N(0x00010, int8_t, ClientID);
            DEFINE_MEMBER_N(0x00011, int8_t, Team);
            DEFINE_MEMBER_N(0x0012, char, Name[14]);
            DEFINE_MEMBER_N(0x0020, pCharacterFx*, characFX);
            DEFINE_MEMBER_N(0x0028, int32_t, ScoreBoard);
            DEFINE_MEMBER_N(0x002C, int32_t, BOTorNOT);
            DEFINE_MEMBER_N(0x0030, int32_t, C4);
            DEFINE_MEMBER_N(0x0034, int32_t, Host);
            DEFINE_MEMBER_N(0x0038, int64_t, Rank);
            DEFINE_MEMBER_N(0x0040, int64_t, IDNumber);
            DEFINE_MEMBER_N(0x0048, int32_t, Spectator);
            DEFINE_MEMBER_N(0x004C, int32_t, Health);
            DEFINE_MEMBER_N(0x0050, int8_t, Kills);
            DEFINE_MEMBER_N(0x0051, int8_t, Deaths);
        };
    };

    class pCharacterFx
    {
    public:
        union
        {
            DEFINE_MEMBER_0(void*, ObjectFX);
            DEFINE_MEMBER_N(0x0760, int8_t, isDead);
            DEFINE_MEMBER_N(0x0708, CCharacterHitBox*, pCharacterHitBox);
            DEFINE_MEMBER_N(0x17BD0, CWeapon*, WeaponOnHand);
        };
    };

     
    

    class CWeapon
    {
    public:
        union {
            DEFINE_MEMBER_0(int16_t, WeaponID);
            DEFINE_MEMBER_N(0x0002, WeaponType, WeaponClass);
            DEFINE_MEMBER_N(0x0010, char, WeaponName[33]);
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
            DEFINE_MEMBER_N(0x0010, D3DXVECTOR3, camerapos);

        };

    };


    class LTClientShell
    {
    public:
        union
        {
            char Name[14];
            DEFINE_MEMBER_N(0x0040, pCamera*, CCamera);
            DEFINE_MEMBER_N(0x0088, void*, CWorldPropsClnt);
            DEFINE_MEMBER_N(0x0460, pPlayerClntBase*, CPlayerClntBase);
            DEFINE_MEMBER_N(0x04F4, int32_t, ingame);
            DEFINE_MEMBER_N(0x04DC, int32_t, Local_ID);
            DEFINE_MEMBER_N(0x00B0, void*, CLTClient);
            DEFINE_MEMBER_N(0x0190, void*, CameraInstance);
            DEFINE_MEMBER_N(0x02B0, void*, CGameUI);
            DEFINE_MEMBER_N(0x02DC, int32_t, Health);
            DEFINE_MEMBER_N(0x010, void*, EntityStart);
        };

        inline uintptr_t PlayerPtrAddr(int i) {
          
          
            return offs::LT_SHELL + ENTITY_START + size_t(i) * 8;
        }

        KLASSES::pPlayer GetPlayerByIndex(int i) {
			 
            uintptr_t p = mem.Read<uintptr_t>(PlayerPtrAddr(i));
            if (!p) return {};
            
            return mem.Read<KLASSES::pPlayer>(p);
        }
        pPlayer GetLocalPlayer(Memory& memRef)
        {
           
            int localId = this->Local_ID;
            if (localId < 0)
                return {};
            for (int i = 0; i < MAX_PLAYERS; ++i)
            {
                KLASSES::pPlayer pl = GetPlayerByIndex(i);
                if (!pl.hObject) continue;

                if (pl.ClientID == localId)
                    return pl;
            }

            return {};
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
            DEFINE_MEMBER_N(0x01BC, D3DXVECTOR3, AbsolutePosition);
            DEFINE_MEMBER_N(0x2978, pBoneArray*, BoneArray);
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
            DEFINE_MEMBER_N(0x03E0, int32_t, Ammo);
            DEFINE_MEMBER_N(0x03E4, int32_t, MaxAmmo);
            DEFINE_MEMBER_N(0x0400, void*, PlayerViewManager);
            DEFINE_MEMBER_N(0x1E28, float, Yaw2);
            DEFINE_MEMBER_N(0x1EA0, float, Pitch);
            DEFINE_MEMBER_N(0x1EA4, float, Yaw);
            DEFINE_MEMBER_N(0x13B8, float, Recoil1);
            DEFINE_MEMBER_N(0x1C08, float, Recoil2);
            DEFINE_MEMBER_N(0x0DF8, D3DXVECTOR2, ViewAngles);
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




    class BasicPlayerinfo
    {
    public:
        char pad_0000[4]; //0x0000
        float MovementSpeed; //0x0004
        float MovementWalkRate; //0x0008
        float MovementDuckWalkRate; //0x000C
        float MovementSideMoveRate; //0x0010
        float MovementFrontBackRunAnimationRate; //0x0014
        float MovementLeftRightWalkAnimationRate; //0x0018
        float MovementAcceleration; //0x001C
        float MovementLRWalkAnimRate; //0x0020
        float MovementAccelation; //0x0024
        float MovementFriction; //0x0028
        float JumpTime; //0x002C
        float JumpVelocity; //0x0030
        float JumpLandedWaitTime; //0x0034
        float JumpLandedNoJumpTimeRate; //0x0038
        float JumpRepeatPenaltyMoveRate; //0x003C
        float JumpRepeatPenaltyHeightRate; //0x0040
        float JumpLandedMovePenaltyTimeRate; //0x0044
        float JumpLandedMovePenaltyMoveRate; //0x0048
        char N00004AAF[40]; //0x004C
        float DamagePenaltyTime; //0x0074
        float DamagePenaltyMoveRate; //0x0078
        float C4Plant; //0x007C
        float C4DEFUSE; //0x0080
        float MaxCanDefuseDistance; //0x0084
        float CharacterHiddenAlpha; //0x0088
        float CharacterHiddenWalkAlpha; //0x008C
        char pad_0100[152]; //0x0100

    }; //Size: 0x0198





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