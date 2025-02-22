#pragma once

#include <thread>
#include <chrono>
#include "Memory//Memory.h"
#include "offsets.h"
#include "Overlay.h"
#include <iostream>
#include <atomic>
#include <condition_variable>
#include <Classes.h>
#include "Memory.h"
#include "Cache.h"
#include "CacheManager.h"

class ESPManager {
public:

    static ESPManager& GetInstance() {
        static ESPManager instance;
        return instance;
    }

    ESPManager(const ESPManager&) = delete;
    ESPManager& operator=(const ESPManager&) = delete;

    void ToggleESP(Memory& mem);

    bool IsESPActive() const {
        return m_active.load(std::memory_order_relaxed);
    }

    ESPManager() = default;
    ~ESPManager() = default;

    void StartESP(Memory& mem);
    void StopESP();

    void ESPWorker(std::stop_token stopToken, Memory& mem);

    void DrawPlayerESP(Memory& mem, const std::shared_ptr<ESP::Snapshot>& snapshot);

    std::atomic<bool> m_active{ false };
    std::jthread      m_thread;
};

 
/*/DrawRect(static_cast<int>(headPos.x - width / 2),
               static_cast<int>(headPos.y),
               static_cast<int>(width),
               static_cast<int>(height),
               &ESPColorEnemy, 1);*/


//pCharacterFx Character = mem.Read<pCharacterFx>((uintptr_t)player.characFX);
           //auto Character = player.characFX;
           //ILTClient client(mem);
           //client.TestGlowESP(mem, reinterpret_cast<obj*>(Character->pObjectFx));
