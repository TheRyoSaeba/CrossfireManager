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
        std::this_thread::sleep_for(8ms);
    }
}

void KLASSES::AimbotManager::AimAtTarget(Memory& mem, const std::shared_ptr<ESP::Snapshot>& snapshot) {
    if (snapshot->localIsDead != 0)
        return;
    int moveX = 0, moveY = 0;
    if (Movement(snapshot, mem, moveX, moveY)) {
        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
            return;
        if (moveX != 0)
            kmBoxBMgr.lock_my();
        if (moveY != 0)
            kmBoxBMgr.lock_mx();
        kmBoxBMgr.km_move_auto(moveX, moveY,2);
        kmBoxBMgr.unlock_mx();
        kmBoxBMgr.unlock_my();
    }
}

 
            







 



    