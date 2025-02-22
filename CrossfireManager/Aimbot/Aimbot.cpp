#include "Aimbot.hpp"
#include <algorithm>
#include <thread>
#include "KMBOX.h"
#include <Cache.h>
#include <cmath>
#include <iostream>
#include "AimHelpers.h"
#include "CacheManager.h"
CacheManager g_cacheManager;


using namespace KLASSES;

void AimbotManager::ToggleAimbot(Memory& mem) {
    const bool newState = !m_active.load(std::memory_order_relaxed);
    if (newState) {
        StartAimbot(mem);
    }
    else {
        StopAimbot();
    }
    Utils::DebugLog("[Aimbot] %s", newState ? "Enabled" : "Disabled");
}

void AimbotManager::StartAimbot(Memory& mem) {
    m_active.store(true, std::memory_order_relaxed);

     
    g_cacheManager.StartUpdateThread(mem);

    m_thread = std::jthread([this, &mem](std::stop_token stopToken) {
        AimbotWorker(stopToken, mem);
        });
}

void AimbotManager::StopAimbot() {
    m_active.store(false, std::memory_order_relaxed);
    if (m_thread.joinable()) {
        m_thread.request_stop();
        m_thread.join();
    }
}

void AimbotManager::AimbotWorker(std::stop_token stopToken, Memory& mem) {
    while (!stopToken.stop_requested() && m_active.load(std::memory_order_relaxed)) {
         
        auto snapshot = g_cacheManager.GetSnapshot();
        AimAtTarget(mem, snapshot);
        std::this_thread::sleep_for(5ms);
    }
}

void KLASSES::AimbotManager::AimAtTarget(Memory& mem, const std::shared_ptr<ESP::Snapshot>& snapshot)
{
    if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
        return;

    if (snapshot->localAbsPos.x == 0 && snapshot->localAbsPos.y == 0 && snapshot->localAbsPos.z == 0)
        return;

    int moveX = 0, moveY = 0;

    if (Movement(snapshot, mem, moveX, moveY))
    {
        Utils::DebugLog("[AimAtTarget] Movement: X: %d, Y: %d", moveX, moveY);
        kmBoxBMgr.lock_mx();
        kmBoxBMgr.lock_my();
        kmBoxBMgr.km_move(moveX, moveY);
        

    }
}

 
 

 
 
 
 


 
   /*float deltaYaw = desiredAngles.y - localYaw;
     float deltaPitch = desiredAngles.x - localPitch;
    if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
    else if (deltaYaw < -180.0f) deltaYaw += 360.0f;
     deltaYaw = -deltaYaw;
    Utils::DebugLog("[RawDeltas] Yaw: %.2f, Pitch: %.2f", deltaYaw, deltaPitch);
    desiredAngles.x = std::clamp(desiredAngles.x, -1539.0f, 1539.0f);
    Utils::DebugLog("[DesiredAngles] Yaw: %.2f, Pitch: %.2f", desiredAngles.y, desiredAngles.x);
    const float YAW_MULTIPLIER = 0.15f;   
    const float PITCH_MULTIPLIER = 0.04f;  
    int moveX = static_cast<int>(deltaYaw * YAW_MULTIPLIER);
    int moveY = static_cast<int>(deltaPitch * PITCH_MULTIPLIER);

    
    if (abs(moveX) < 2) moveX = 0;
    if (abs(moveY) < 2) moveY = 0;

    if (moveX != 0 || moveY != 0) {
        MoveMouse(moveX, moveY);
    }
    Utils::DebugLog("[Movement] dYaw: %.2f → X: %d | dPitch: %.2f → Y: %d",
        deltaYaw, moveX, deltaPitch, moveY);*/   
 

 





 



