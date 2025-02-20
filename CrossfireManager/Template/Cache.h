#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include "offsets.h"
#include <array>
#include <mutex>
#include <chrono>
#include <Classes.h> 
#include "Memory.h"   
#include <cstddef> 
#include <d3dx9math.h> 

namespace ESP {
    constexpr int MAX_PLAYERS = 16;

    struct Cache {

        std::shared_ptr<KLASSES::LTClientShell> clientShell;
        std::shared_ptr<KLASSES::pPlayer> localPlayer;
        std::vector<std::shared_ptr<KLASSES::pPlayer>> players;
        std::array<std::shared_ptr<D3DXVECTOR3>, MAX_PLAYERS> headPositions;
        std::array<std::shared_ptr<D3DXVECTOR3>, MAX_PLAYERS> footPositions;
        std::array<std::shared_ptr<int8_t>, MAX_PLAYERS> isDeadFlags;

        std::chrono::steady_clock::time_point lastCacheUpdate;
        mutable std::mutex cacheMutex;

        Cache() {
            clientShell = std::make_shared<KLASSES::LTClientShell>();
            localPlayer = std::make_shared<KLASSES::pPlayer>();
            players.reserve(MAX_PLAYERS);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                players.emplace_back(std::make_shared<KLASSES::pPlayer>());
                headPositions[i] = std::make_shared<D3DXVECTOR3>();
                footPositions[i] = std::make_shared<D3DXVECTOR3>();
                isDeadFlags[i] = std::make_shared<int8_t>(0);
            }
            lastCacheUpdate = std::chrono::steady_clock::now();
        }

        bool UpdateClientShell(Memory& mem, uintptr_t LT_SHELL) {
            std::lock_guard<std::mutex> lock(cacheMutex);
            return mem.Read(LT_SHELL, clientShell.get(), sizeof(KLASSES::LTClientShell));
        }

        void UpdateLocalPlayer(Memory& mem) {
            std::lock_guard<std::mutex> lock(cacheMutex);
            *localPlayer = clientShell->GetLocalPlayer(mem);
        }

        void UpdatePlayers(Memory& mem) {
            std::lock_guard<std::mutex> lock(cacheMutex);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                *players[i] = clientShell->GetPlayerByIndex(i);
            }
        }

        void UpdatePlayerData(Memory& mem, VMMDLL_SCATTER_HANDLE scatterHandle) {
            std::lock_guard<std::mutex> lock(cacheMutex);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                const auto& p = players[i];
                if (!p->hObject || p->Team == localPlayer->Team || p->ClientID == localPlayer->ClientID)
                    continue;

                uintptr_t playerAddr = reinterpret_cast<uintptr_t>(p->hObject);
                mem.AddScatterReadRequest(
                    scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, Head),
                    headPositions[i].get(),
                    sizeof(D3DXVECTOR3)
                );
                mem.AddScatterReadRequest(
                    scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, foot),
                    footPositions[i].get(),
                    sizeof(D3DXVECTOR3)
                );
                mem.AddScatterReadRequest(
                    scatterHandle,
                    reinterpret_cast<uintptr_t>(p->characFX) + offsetof(KLASSES::pCharacterFx, isDead),
                    isDeadFlags[i].get(),
                    sizeof(int8_t)
                );
            }
        }

        void Clear() {
            std::lock_guard<std::mutex> lock(cacheMutex);
            clientShell.reset();
            localPlayer.reset();
            players.clear();
            headPositions.fill(nullptr);
            footPositions.fill(nullptr);
            isDeadFlags.fill(nullptr);
        }

        bool IsValid() const {
            return clientShell != nullptr && localPlayer != nullptr;
        }

        void UpdateTimestamp() {
            std::lock_guard<std::mutex> lock(cacheMutex);
            lastCacheUpdate = std::chrono::steady_clock::now();
        }

        auto GetTimeSinceLastUpdate() const {
            std::lock_guard<std::mutex> lock(cacheMutex);
            return std::chrono::steady_clock::now() - lastCacheUpdate;
        }
    };
}

#endif 