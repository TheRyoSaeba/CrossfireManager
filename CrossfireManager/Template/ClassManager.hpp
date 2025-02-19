#ifndef CLASS_MANAGER_HPP
#define CLASS_MANAGER_HPP

#include "Memory.h"
#include "Classes.h"
#include <thread>
#include <atomic>
#include <memory>
#include "AimbotManager.hpp"  
#include "ESPManager.hpp"        


    template <typename Feature>
    class FeatureManager {
    public:
        static FeatureManager& GetInstance() {
            static FeatureManager instance;
            return instance;
        }

        FeatureManager(const FeatureManager&) = delete;
        FeatureManager& operator=(const FeatureManager&) = delete;

        void ToggleFeature(Memory& mem, std::shared_ptr<GameState> gameState) {
            if (m_active.load(std::memory_order_relaxed)) {
                StopFeature();
            }
            else {
                StartFeature(mem, gameState);
            }
        }

        bool IsFeatureActive() const {
            return m_active.load(std::memory_order_relaxed);
        }

        void StartFeature(Memory& mem, std::shared_ptr<GameState> gameState) {
            if (m_active.load(std::memory_order_relaxed)) return;
            m_active.store(true, std::memory_order_relaxed);
            m_thread = std::jthread(&FeatureManager::FeatureWorker, this, std::ref(mem), gameState);
        }

        void StopFeature() {
            m_active.store(false, std::memory_order_relaxed);
            if (m_thread.joinable()) m_thread.request_stop();
        }

    private:
        std::atomic<bool> m_active{ false };
        std::jthread      m_thread;

        void FeatureWorker(std::stop_token stopToken, Memory& mem, std::shared_ptr<GameState> gameState) {
            Feature feature;
            while (!stopToken.stop_requested()) {
                feature.Run(mem, gameState);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }
    };

    class AimbotFeature {
    public:
        AimbotFeature() = default;  // Add default constructor
        void Run(Memory& mem, std::shared_ptr<GameState> gameState) {
            AimbotManager::GetInstance().AimAtTarget(mem, gameState);
        }
    };

    class ESPFeature {
    public:
        ESPFeature() = default;  // Add default constructor
        void Run(Memory& mem, std::shared_ptr<GameState> gameState) {
           ESPManager::GetInstance().DrawPlayerESP(mem, gameState);
        }
    };

    using AimbotFeatureManager = FeatureManager<AimbotFeature>;
    using ESPFeatureManager = FeatureManager<ESPFeature>;


#endif // CLASS_MANAGER_HPP