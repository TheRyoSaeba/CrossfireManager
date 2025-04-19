#pragma once

#include "Cache.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>



class CacheManager {
public:
    CacheManager()
        : m_running(false),
        m_minCycle(std::chrono::milliseconds(5))
      
    {
        m_activeCache = std::make_unique<ESP::Cache>();
        m_pendingCache = std::make_unique<ESP::Cache>();
        m_activeSnapshot.store(std::make_shared<ESP::Snapshot>(m_activeCache->GetSnapshot()));
        m_lastCycle = std::chrono::steady_clock::now();
    }

    ~CacheManager() {
        StopUpdateThread();
    }

    void Update(Memory& mem) {
        auto targetTime = m_lastCycle + m_minCycle;

        {
            std::lock_guard<std::mutex> lock(m_updateMutex);
            mem.Read(offs::LT_SHELL, &m_activeCache->m_clientShell, sizeof(KLASSES::LTClientShell));
        }

        bool gameValid = m_pendingCache->Update(mem);

        {
            std::lock_guard<std::mutex> lock(m_updateMutex);
            if (gameValid) {
                std::swap(m_activeCache, m_pendingCache);
            }
            m_activeSnapshot.store(std::make_shared<ESP::Snapshot>(m_activeCache->GetSnapshot()), std::memory_order_release);
        }

        auto now = std::chrono::steady_clock::now();
        if (now < targetTime) {
            std::this_thread::sleep_until(targetTime);
        }
        m_lastCycle = now;
    }

    std::shared_ptr<ESP::Snapshot> GetSnapshot() const {
        return m_activeSnapshot.load();
    }

    void StartUpdateThread(Memory& mem) {
        if (m_running.load()) return;
        m_running.store(true);
        m_updateThread = std::thread([this, &mem]() {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

            while (m_running.load()) {
                auto start = std::chrono::steady_clock::now();

                this->Update(mem);

                auto end = std::chrono::steady_clock::now();
                auto updateTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                int sleepTime = std::max(1, 9 - static_cast<int>(updateTime));
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
            });
    }

    void StopUpdateThread() {
        m_running.store(false);
        if (m_updateThread.joinable()) {
            m_updateThread.join();
        }
    }

private:
    std::unique_ptr<ESP::Cache> m_activeCache;
    std::unique_ptr<ESP::Cache> m_pendingCache;
    std::atomic<std::shared_ptr<ESP::Snapshot>> m_activeSnapshot;
    mutable std::mutex m_updateMutex;
    std::chrono::milliseconds m_minCycle;
    std::chrono::steady_clock::time_point m_lastCycle;
    std::chrono::milliseconds m_targetCycle;
    std::atomic<bool> m_running;
    std::thread m_updateThread;
};

class CacheManager;  
extern CacheManager g_cacheManager;  