#pragma once
#include "Classes.h"
#include <d3dx9math.h>
#include <Windows.h>
#include "Cache.h"
#include <cmath>
#include <algorithm>
#include <float.h>
#include "Aimbot.hpp"
#include "KMBOX.h"

namespace KLASSES {
    struct TargetInfo {
        const ESP::MinimalPlayerData* playerData;
        float distance;
    };

    struct TargetInfo2 {
        const ESP::MinimalPlayerData* playerData;
        float screenDistance;
    };

    inline TargetInfo FindBestTarget(
        const D3DXVECTOR3& localAbsPos,
        const std::vector<ESP::MinimalPlayerData>& enemies
    ) {
        TargetInfo bestTarget{};
        float closestDistance = FLT_MAX;

        for (const auto& enemy : enemies) {
            if (enemy.IsDead) continue;
            D3DXVECTOR3 delta = enemy.AbsPos - localAbsPos;
            float distance = D3DXVec3Length(&delta);
            if (distance < closestDistance) {
                closestDistance = distance;
                bestTarget.playerData = &enemy;
                bestTarget.distance = distance;
            }
        }
        return bestTarget;
    }


    inline const ESP::MinimalPlayerData* FindClosestTarget(
        const D3DXVECTOR3& localAbsPos,
        const std::vector<ESP::MinimalPlayerData>& enemies)
    {
        const ESP::MinimalPlayerData* bestTarget = nullptr;
        float bestWorldDistance = FLT_MAX;
        for (const auto& enemy : enemies) {
            if (enemy.IsDead)
                continue;
            D3DXVECTOR3 delta = enemy.AbsPos - localAbsPos;
            float worldDistance = D3DXVec3Length(&delta);
            if (worldDistance < bestWorldDistance) {
                bestWorldDistance = worldDistance;
                bestTarget = &enemy;
            }
        }
        return bestTarget;
    }





    inline float CalculateDynamicSmooth(float distance, float maxDistance = 5000.0f, float baseSmooth = 0.2f) {
        return baseSmooth * std::clamp(distance / maxDistance, 0.1f, 1.0f);
    }

    inline D3DXVECTOR3 CalcAngleWorld(const D3DXVECTOR3& src, const D3DXVECTOR3& dst) {
        D3DXVECTOR3 delta = dst - src;
        float distance = D3DXVec3Length(&delta);


        float pitch = -asinf(delta.y / distance) * (1539.0f / static_cast<float>(M_PI_2));


        float yaw = atan2f(delta.x, delta.z) * (180.0f / static_cast<float>(M_PI));

        return { pitch, yaw, 0.0f };
    }

    inline bool Movement(const std::shared_ptr<ESP::Snapshot>& snapshot, Memory& mem, int& moveX, int& moveY)
    {
        if (snapshot->enemies.empty())
            return false;

        const D3DXVECTOR3& localAbsPos = snapshot->localAbsPos;
        const ESP::MinimalPlayerData* target = FindClosestTarget(localAbsPos, snapshot->enemies);
        if (!target)
            return false;

        LT_DRAWPRIM drawPrim = mem.Read<LT_DRAWPRIM>(offs::ILTDrawPrim);
        const int screenCenterX = drawPrim.viewport.X + (drawPrim.viewport.Width / 2);
        const int screenCenterY = drawPrim.viewport.Y + (drawPrim.viewport.Height / 2);

        D3DXVECTOR3 screenHead = target->HeadPos;
        D3DXVECTOR3 screenFoot = target->FootPos;
        if (!EngineW2S(drawPrim, &screenHead) || !EngineW2S(drawPrim, &screenFoot))
            return false;

        D3DXVECTOR3 bodyCenter = {
            (screenHead.x + screenFoot.x) / 2.0f,
            (screenHead.y + screenFoot.y) / 2.0f,
            0.0f
        };
         
        D3DXVECTOR3 delta = target->AbsPos - localAbsPos;

        const float worldDistance = D3DXVec3Length(&delta);

        const D3DXMATRIX& projMatrix = drawPrim.projection;


        const float verticalFOV = 2.0f * atanf(1.0f / projMatrix.m[1][1]); 

        const float viewportHeight = static_cast<float>(drawPrim.viewport.Height);
        const float pixelsPerMeter = (viewportHeight / (2.0f * tanf(verticalFOV / 2.0f)));

        const float REAL_HEAD_HEIGHT = 0.7f;
        const float verticalOffset = (REAL_HEAD_HEIGHT / worldDistance) * pixelsPerMeter;

         
        D3DXVECTOR3 adjustedAim = {
        bodyCenter.x,
        bodyCenter.y - verticalOffset, 
        0.0f
        };

        moveX = static_cast<int>((adjustedAim.x - screenCenterX) * 0.1f);
        moveY = static_cast<int>((adjustedAim.y - screenCenterY) * 0.1f);
        return true;
    }



     






    

    float NormalizeYaw(float yaw) {
        // Normalize yaw to [0, 360)
        yaw = fmod(yaw, 360.0f);
        if (yaw < 0.0f) yaw += 360.0f;
        return yaw;
    }
    inline void MoveMouse(int dx, int dy) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

    
    }

