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

    enum class EnemyPosition : uint8_t
    {
        INSIDE_SCREEN = 0,
        BEHIND = 1 << 0,
        ABOVE = 1 << 1,
        BELOW = 1 << 2,
        LEFT = 1 << 3,
        RIGHT = 1 << 4
    };

    struct TargetInfo
    {
        const ESP::MinimalPlayerData* playerData;
        float distance;
    };

    struct TargetInfo2
    {
        const ESP::MinimalPlayerData* playerData;
        float screenDistance;
    };

    inline float GetHorizontalFOV(const D3DXMATRIX& projMatrix, float aspectRatio)
    {
        float verticalFOV = 2.0f * atanf(1.0f / projMatrix.m[1][1]);
        return 2.0f * atanf(tanf(verticalFOV / 2.0f) * aspectRatio) * (180.0f / 3.14159f);
    }

    inline float CalculateDynamicSmooth(float distance, float maxDistance = 5000.0f, float baseSmooth = 0.2f)
    {
        return baseSmooth * std::clamp(distance / maxDistance, 0.1f, 1.0f);
    }

    inline TargetInfo FindBestTarget(
        const D3DXVECTOR3& localAbsPos,
        const std::vector<ESP::MinimalPlayerData>& enemies)
    {
        TargetInfo bestTarget{};
        float closestDistance = FLT_MAX;

        for (const auto& enemy : enemies)
        {
            if (enemy.IsDead)
                continue;

            D3DXVECTOR3 delta = enemy.AbsPos - localAbsPos;
            float distance = D3DXVec3Length(&delta);

            if (distance < closestDistance)
            {
                closestDistance = distance;
                bestTarget.playerData = &enemy;
                bestTarget.distance = distance;
            }
        }

        return bestTarget;
    }

    inline const ESP::MinimalPlayerData* FindClosestTargetFOV(
        const D3DXVECTOR3& localAbsPos,
        const std::vector<ESP::MinimalPlayerData>& enemies,
        const LT_DRAWPRIM& drawPrim)
    {
        const ESP::MinimalPlayerData* bestTarget = nullptr;
        float bestScreenDistance = FLT_MAX;
        int screenCenterX = drawPrim.viewport.X + drawPrim.viewport.Width / 2;
        int screenCenterY = drawPrim.viewport.Y + drawPrim.viewport.Height / 2;

        for (const auto& enemy : enemies)
        {
            if (enemy.IsDead)
                continue;

            D3DXVECTOR3 screenPos = enemy.AbsPos;
            if (!EngineW2S(drawPrim, &screenPos))
                continue;

            float dx = screenPos.x - screenCenterX;
            float dy = screenPos.y - screenCenterY;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < bestScreenDistance)
            {
                bestScreenDistance = dist;
                bestTarget = &enemy;
            }
        }

        return bestTarget;
    }

    inline D3DXVECTOR3 CalcAngleWorld(
        const D3DXVECTOR3& src,
        const D3DXVECTOR3& dst)
    {
        D3DXVECTOR3 delta = dst - src;
        float distance = D3DXVec3Length(&delta);
        float pitch = -asinf(delta.y / distance) * (1539.0f / static_cast<float>(M_PI_2));
        float yaw = atan2f(delta.x, delta.z) * (180.0f / static_cast<float>(M_PI));

        return { pitch, yaw, 0.0f };
    }

    float NormalizeYaw(float yaw)
    {
        yaw = fmod(yaw, 360.0f);
        if (yaw < 0.0f)
            yaw += 360.0f;
        return yaw;
    }

    inline bool Movement(
        const std::shared_ptr<ESP::Snapshot>& snapshot,
        Memory& mem,
        int& moveX,
        int& moveY)
    {
        if (snapshot->enemies.empty())
            return false;

        LT_DRAWPRIM drawPrim = mem.Read<LT_DRAWPRIM>(offs::ILTDrawPrim);
        const D3DXVECTOR3& localAbsPos = snapshot->localAbsPos;
        float aspectRatio = static_cast<float>(drawPrim.viewport.Width) / drawPrim.viewport.Height;

        const ESP::MinimalPlayerData* target = FindClosestTargetFOV(localAbsPos, snapshot->enemies, drawPrim);
        if (!target || target->IsDead != 0)
            return false;

        int screenCenterX = drawPrim.viewport.X + (drawPrim.viewport.Width / 2);
        int screenCenterY = drawPrim.viewport.Y + (drawPrim.viewport.Height / 2);

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
        float worldDistance = D3DXVec3Length(&delta) / 100.0f;
        const D3DXMATRIX& projMatrix = drawPrim.projection;
        float verticalFOV = 2.0f * atanf(1.0f / projMatrix.m[1][1]);
        float viewportHeight = static_cast<float>(drawPrim.viewport.Height);
        float pixelsPerMeter = viewportHeight / (2.0f * tanf(verticalFOV / 2.0f));
        const float REAL_HEAD_HEIGHT = 0.9f;
        const float MIN_OFFSET = 15.0f;
        float verticalOffset = (REAL_HEAD_HEIGHT / worldDistance) * pixelsPerMeter;
        verticalOffset = std::max(verticalOffset, MIN_OFFSET);

        D3DXVECTOR3 adjustedAim = { bodyCenter.x, bodyCenter.y - verticalOffset, 0.0f };

        Utils::DebugLog("[AimDebug] BodyCenterY: %.1f | Offset: %.1f | FinalY: %.1f",
            bodyCenter.y, verticalOffset, adjustedAim.y);
        Utils::DebugLog("[Offset] WorldDist: %.1fm | Pixels/M: %.1f | Offset: %.1f",
            worldDistance, pixelsPerMeter, verticalOffset);

        moveX = static_cast<int>((adjustedAim.x - screenCenterX) * 0.1f);
        moveY = static_cast<int>((adjustedAim.y - screenCenterY) * 0.1f);

        return true;
    }

    inline void MoveMouse(int dx, int dy)
    {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

} // namespace KLASSES
