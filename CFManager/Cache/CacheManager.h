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
        m_isUpdating.store(true, std::memory_order_release);
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
        m_isUpdating.store(false, std::memory_order_release);
    }

    std::shared_ptr<ESP::Snapshot> GetSnapshot() const {
        return m_activeSnapshot.load();
    }

    void StartUpdateThread(Memory& mem) {
        if (m_running.load()) return;
        m_running.store(true);
        m_updateThread = std::thread([this, &mem]() {
            // Keep update thread lower priority so it doesn't starve the UI thread
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
            while (m_running.load()) {
                try {
                    // Update() internally paces to m_minCycle using sleep_until
                    this->Update(mem);
                }
                catch (const std::exception& e) {
                    Logger::Error(std::string("Exception in Cache update thread: ") + e.what());
                }
                catch (...) {
                    Logger::Error("Unknown exception in Cache update thread; continuing");
                }
            }
            });
    }

    bool IsUpdating() const {
        return m_isUpdating.load(std::memory_order_acquire);
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
    std::atomic<bool> m_isUpdating{ false };
};

class CacheManager;  
extern CacheManager g_cacheManager;  