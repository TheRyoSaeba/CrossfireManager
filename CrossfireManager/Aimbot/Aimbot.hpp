#pragma once

#include <cmath>       / 
#include <chrono>
#include <thread>
#include <atomic>   " 
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
#include <cmath>
#include <iostream>

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


        AimbotManager() = default;
        ~AimbotManager() = default;

        void StartAimbot(Memory& mem);
        void StopAimbot();

        void AimbotWorker(std::stop_token stopToken, Memory& mem);

        void AimAtTarget(Memory& mem, const std::shared_ptr<ESP::Snapshot>& snapshot);


        std::atomic<bool> m_active{ false };
        std::jthread      m_thread;
    };

   // D3DXVECTOR3 CalcAngle(D3DXVECTOR3 src, D3DXVECTOR3 dst);
    D3DXVECTOR3 CalcAngleWorld(const D3DXVECTOR3& src, const D3DXVECTOR3& dst);
    void MoveMouse(int dx, int dy);
    void TestSnapAim(Memory& mem);


}