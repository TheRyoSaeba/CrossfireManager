#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <atomic>

#include <DirectXMath.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.h>

#include "Classes.h"
#include "Memory.h"
#include "Cache.h"
#include "CacheManager.h"

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

using namespace std::chrono_literals;

namespace KLASSES {

    class AimbotManager {
    public:
        static AimbotManager& GetInstance() {
            static AimbotManager instance;
            return instance;
        }

        AimbotManager(const AimbotManager&) = delete;
        AimbotManager& operator=(const AimbotManager&) = delete;

        void ToggleAimbot(Memory& mem);

        bool IsAimbotActive() const {
            return m_active.load(std::memory_order_relaxed);
        }

        void StartAimbot(Memory& mem);
        void StopAimbot();

        void AimbotWorker(std::stop_token stopToken, Memory& mem);
        void AimAtTarget(Memory& mem, const std::shared_ptr<ESP::Snapshot>& snapshot);

    private:
        AimbotManager() = default;
        ~AimbotManager() = default;

        std::atomic<bool> m_active{ false };
        std::jthread m_thread;
    };

    D3DXVECTOR3 CalcAngleWorld(const D3DXVECTOR3& src, const D3DXVECTOR3& dst);
    void MoveMouse(int dx, int dy);
    //void TestSnapAim(Memory& mem);

} // namespace KLASSES
