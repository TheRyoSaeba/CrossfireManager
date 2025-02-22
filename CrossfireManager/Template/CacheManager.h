#pragma once

#include "Cache.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>

class CacheManager {
public:
    CacheManager()
        : m_running(false),
        m_minCycle(std::chrono::milliseconds(8)) 
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
        auto startTime = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> updateLock(m_updateCallMutex);

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastCycle);
        if (elapsed < m_minCycle) {
            std::this_thread::sleep_for(m_targetCycle - elapsed);
        }
        m_lastCycle = std::chrono::steady_clock::now();

        if (!m_pendingCache->Update(mem)) {
            return;
        }
        ESP::Snapshot newSnapshot = m_pendingCache->GetSnapshot();

        {
            std::lock_guard<std::mutex> lock(m_updateMutex);
            m_activeSnapshot.store(std::make_shared<ESP::Snapshot>(newSnapshot));
            std::swap(m_activeCache, m_pendingCache);
        }
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        KLASSES::Utils::DebugLog("[CacheManager] Update took %lld ms", duration);
    }

    std::shared_ptr<ESP::Snapshot> GetSnapshot() const {
        return m_activeSnapshot.load();
    }

    void StartUpdateThread(Memory& mem) {
        if (m_running.load()) return; 
        m_running.store(true);
        m_updateThread = std::thread([this, &mem]() {
            while (m_running.load()) {
                this->Update(mem);
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
    
    std::mutex m_updateCallMutex;
    std::chrono::milliseconds m_minCycle;
    std::chrono::steady_clock::time_point m_lastCycle;
    std::chrono::milliseconds m_targetCycle;
   

    std::atomic<bool> m_running;
    std::thread m_updateThread;
};

extern CacheManager g_cacheManager;