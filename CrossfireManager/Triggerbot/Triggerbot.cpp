/*
    ================================================================================
    FILE: triggerbot.cpp
    DESC: Implements the “triggerbot” logic declared in triggerbot.h
          - CalcAngle
          - IsEnemyInCrosshair
          - Triggerbot main loop
    ================================================================================
*/

#include "triggerbot.h"
#include <algorithm>    // std::fmod, std::remove, etc.
#include <numbers>      // if you need std::numbers::pi (C++20)
#include <cmath>        // for std::sqrt, std::atan2, std::fabs
#include <thread>       // std::this_thread::sleep_for  // if needed for mem usage
#include <cstdio>
#include "Classes.h"
 

namespace KLASSES {
     
    namespace Triggerbot {

        /*
            ----------------------------------------------------------------------------
            1) CalcAngle()
            ----------------------------------------------------------------------------
        */
        D3DXVECTOR3 CalcAngle(D3DXVECTOR3 src, D3DXVECTOR3 dst) {
            D3DXVECTOR3 delta = {
                dst.x - src.x,  // Left/Right
                dst.y - src.y,  // Up/Down
                dst.z - src.z   // Forward/Backward
            };

            // ✅ Pitch Calculation (Crossfire Uses -1539 to 1539)
            float hypotenuse = std::sqrt(delta.x * delta.x + delta.z * delta.z);  // 2D distance on X/Z plane
            float totalDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);  // Full 3D distance

            D3DXVECTOR3 angles;
            angles.x = -std::asin(delta.y / totalDistance) * (1539.0f / static_cast<float>(M_PI));  // ✅ Fixed for CF’s pitch system
            angles.y = std::atan2(delta.x, delta.z) * (180.0f / static_cast<float>(M_PI));  // ✅ Correct yaw calculation
            angles.z = 0.0f;  // No roll

            return angles;
        }


        /*
            ----------------------------------------------------------------------------
            2) IsEnemyInCrosshair()
            ----------------------------------------------------------------------------
        */
        bool IsEnemyInCrosshair(float localYaw, float localPitch, D3DXVECTOR3 aimAngles) {
            float deltaPitch = aimAngles.x - localPitch;

            // ✅ Normalize yaw for infinite Crossfire yaw system
            float deltaYaw = aimAngles.y - localYaw;
            while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
            while (deltaYaw < -180.0f) deltaYaw += 360.0f;

            // ✅ Adjust FOV thresholds
            float yawThreshold = 3.5f;  // ✅ Increased for better hit consistency
            float pitchThreshold = 100.0f;  // ✅ Adjusted for CF pitch scale

            bool inCrosshair = (std::fabs(deltaYaw) < yawThreshold) && (std::fabs(deltaPitch) < pitchThreshold);

            if (inCrosshair) {
                Utils::DebugLog("🎯 Enemy in Crosshair! ΔYaw: %.2f, ΔPitch: %.2f", deltaYaw, deltaPitch);
            }
            else {
                Utils::DebugLog("❌ Missed - ΔYaw: %.2f, ΔPitch: %.2f", deltaYaw, deltaPitch);
            }

            return inCrosshair;
        }


        /*
            ----------------------------------------------------------------------------
            3) Triggerbot()
            ----------------------------------------------------------------------------
            - Loops indefinitely. Reads local player, scans for enemies in crosshair,
              and clicks using KMBox if found.
        */
        void Triggerbot(Memory& mem) {
            Utils::DebugLog("🔫 Triggerbot Started");

             

            while (true) {
                LTClientShell clientShell = mem.Read<LTClientShell>(LT_SHELL);
                pPlayer localPlayer = clientShell.GetLocalPlayer(mem);

                if (!localPlayer.hObject || localPlayer.Health <= 0) {
                    Utils::DebugLog("❌ Invalid Local Player!");
                    continue;
                }

                Utils::DebugLog("✅ Local Player Found - Health: %d, Team: %d", localPlayer.Name, localPlayer.Team);

                auto playerCltnBase = clientShell.CPlayerClntBase;
                auto yawAddr = reinterpret_cast<uintptr_t>(&playerCltnBase->Yaw);
                auto pitchAddr = reinterpret_cast<uintptr_t>(&playerCltnBase->Pitch);
                float localYaw = mem.Read<float>(yawAddr);
                float localPitch = mem.Read<float>(pitchAddr);
                while (localYaw > 180.0f) localYaw -= 360.0f;
                while (localYaw < -180.0f) localYaw += 360.0f;
                Utils::DebugLog("🎯 Current View Angles -> Yaw: %.2f, Pitch: %.2f", localYaw, localPitch);

                for (int i = 0; i < 16; i++) {
                    pPlayer enemy = clientShell.GetPlayerByIndex(i);
                    if (!enemy.hObject) {
                        Utils::DebugLog("❌ Enemy [%d] has NULL hObject!", i);
                        continue;
                    }
                    if (enemy.Health <= 0 || enemy.Team == localPlayer.Team)
                        continue;

                    Utils::DebugLog("👀 Checking Enemy [%d] - Health: %d, Team: %d", i, enemy.Health, enemy.Team);
                    Utils::DebugLog("Name: %-15s", enemy.Name);

                    
                    D3DXVECTOR3 myheadpos = { 1,3,4 };
                    D3DXVECTOR3 enemyheadpos = { 1,2,3 };

                    D3DXVECTOR3 aimAngles = CalcAngle(myheadpos, enemyheadpos);

                   // Utils::DebugLog("📌 Enemy Position -> X: %.2f, Y: %.2f, Z: %.2f",
                       // enemyHeadPos.x, enemyHeadPos.y, enemyHeadPos.z);

                   // Utils::DebugLog("📐 Calculated Angles -> Yaw: %.2f, Pitch: %.2f", aimAngles.y, aimAngles.x);

                    if (IsEnemyInCrosshair(localYaw, localPitch, aimAngles)) {
                        Utils::DebugLog("🎯 Enemy [%d] In Crosshair! Shooting...", i);
                        //kmBoxBMgr.km_click();
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        break;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }




    } // end namespace Triggerbot
}
 // end namespace KLASSES
