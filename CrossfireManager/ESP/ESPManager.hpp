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
 
 

 
// Include whatever else you need (D3DX, KLASSES, etc.)

class ESPManager {
public:
    // Singleton accessor
    static ESPManager& GetInstance() {
        static ESPManager instance;
        return instance;
    }

    // Non-copyable & non-movable
    ESPManager(const ESPManager&) = delete;
    ESPManager& operator=(const ESPManager&) = delete;

    // Toggling the ESP on/off
    void ToggleESP(Memory& mem);

    // Query current ESP state
    bool IsESPActive() const {
        return m_active.load(std::memory_order_relaxed);
    }

    // Private ctor for singleton pattern
    ESPManager() = default;
    ~ESPManager() = default;

    // Internal start/stop
    void StartESP(Memory& mem);
    void StopESP();

    // Main ESP thread loop 
    void ESPWorker(std::stop_token stopToken, Memory& mem);

    // The actual draw logic for players
    void DrawPlayerESP(Memory& mem);

    // Internal state
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
