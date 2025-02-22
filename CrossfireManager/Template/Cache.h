#pragma once

#include "Classes.h"
#include "Memory.h"
#include "offsets.h"
#include <array>
#include <vector>
#include <mutex>
#include <chrono>
#include <d3dx9math.h>
#include <cstddef>

namespace ESP {

    constexpr int MAX_PLAYERS = 16;

    struct MinimalPlayerData {
        void* hObject;
        int Team;
        int Health;
        char Name[14];
        D3DXVECTOR3 HeadPos;
        D3DXVECTOR3 FootPos;
        D3DXVECTOR3 AbsPos;
        bool IsDead;
    };

    struct Snapshot {
        D3DXVECTOR3 localAbsPos;
        D3DXVECTOR3 localHeadPos;
        std::vector<MinimalPlayerData> enemies;
    };

    class Cache {
    public:
        Cache() : m_scatterHandle(nullptr) {
            Clear();
        }

        ~Cache() {
            if (m_scatterHandle) {

                m_scatterHandle = nullptr;
            }
        }

        bool Update(Memory& mem) {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!ClientUpdate(mem)) {
                return false;
            }

            if (!UpdateLocalPlayer(mem)) {
                return false;
            }

            UpdateEntities(mem);

            if (!UpdatePositions(mem)) {
                return false;
            }

            m_lastUpdate = std::chrono::steady_clock::now();

            return true;
        }

        Snapshot GetSnapshot() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            Snapshot snap;
            snap.localAbsPos = m_localAbsolutePosition;
            snap.localHeadPos = m_localHeadPosition;
            snap.enemies = m_minimalPlayers;
            return snap;
        }

        void Clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_clientShell = KLASSES::LTClientShell{};
            m_localPlayer = KLASSES::pPlayer{};
            m_localAbsolutePosition = D3DXVECTOR3{};
            m_localHeadPosition = D3DXVECTOR3{};
            m_players.fill(KLASSES::pPlayer{});
            m_headPositions.fill(D3DXVECTOR3{});
            m_footPositions.fill(D3DXVECTOR3{});
            m_absolutePositions.fill(D3DXVECTOR3{});
            m_isDeadFlags.fill(0);
            m_minimalPlayers.clear();
             
            m_lastUpdate = std::chrono::steady_clock::now();
            m_lastEntityUpdate = std::chrono::steady_clock::now();
        }

    private:
        mutable std::mutex m_mutex;
        std::chrono::steady_clock::time_point m_lastEntityUpdate;
        static constexpr std::chrono::milliseconds ENTITY_UPDATE_INTERVAL{ 5000 };  
        KLASSES::LTClientShell m_clientShell;
        KLASSES::pPlayer m_localPlayer;
        D3DXVECTOR3 m_localAbsolutePosition;
        D3DXVECTOR3 m_localHeadPosition;

        std::array<KLASSES::pPlayer, MAX_PLAYERS> m_players;
        std::array<D3DXVECTOR3, MAX_PLAYERS> m_headPositions;
        std::array<D3DXVECTOR3, MAX_PLAYERS> m_footPositions;
        std::array<D3DXVECTOR3, MAX_PLAYERS> m_absolutePositions;
        std::array<int8_t, MAX_PLAYERS> m_isDeadFlags;
        std::vector<MinimalPlayerData> m_minimalPlayers;

        std::chrono::steady_clock::time_point m_lastUpdate;

        VMMDLL_SCATTER_HANDLE m_scatterHandle;

        bool ClientUpdate(Memory& mem) {
            auto startTime = std::chrono::steady_clock::now();
            if (!mem.Read(LT_SHELL, &m_clientShell, sizeof(KLASSES::LTClientShell))) {
                return false;
            }
            return true;
             
        }

        bool UpdateLocalPlayer(Memory& mem) {
            
            m_localPlayer = m_clientShell.GetLocalPlayer(mem);
            if (m_localPlayer.hObject != nullptr) {
                m_localAbsolutePosition = mem.Read<D3DXVECTOR3>(
                    reinterpret_cast<uintptr_t>(m_localPlayer.hObject) +
                    offsetof(KLASSES::obj, AbsolutePosition));
                m_localHeadPosition = mem.Read<D3DXVECTOR3>(
                    reinterpret_cast<uintptr_t>(m_localPlayer.hObject) +
                    offsetof(KLASSES::obj, Head));
                return true;
            }
            return false;
            

           
        }

        void UpdateEntities(Memory& mem) {
            auto startTime = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (now - m_lastEntityUpdate < ENTITY_UPDATE_INTERVAL) {
                return;
            }

            uintptr_t ENTITY_BASE = LT_SHELL + offs::dwCPlayerStart;
            size_t bytesToRead = sizeof(KLASSES::pPlayer) * MAX_PLAYERS;

            std::array<KLASSES::pPlayer, MAX_PLAYERS> tempPlayers;

            if (mem.Read(ENTITY_BASE, tempPlayers.data(), bytesToRead)) {
                
                m_players = tempPlayers;
                m_lastEntityUpdate = now;
                KLASSES::Utils::DebugLog("[Cache] Bulk UpdateEntities completed in a single read.");
            }
            else {
                KLASSES::Utils::DebugLog("[Cache] UpdateEntities bulk read failed.");
            }
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            KLASSES::Utils::DebugLog("[CacheManager] Update took %lld ms", duration);
        }
        

        bool UpdatePositions(Memory& mem) {
            
            std::array<D3DXVECTOR3, MAX_PLAYERS> tempHeadPositions;
            std::array<D3DXVECTOR3, MAX_PLAYERS> tempFootPositions;
            std::array<D3DXVECTOR3, MAX_PLAYERS> tempAbsolutePositions;
            std::array<int8_t, MAX_PLAYERS> tempIsDeadFlags{};

            if (!m_scatterHandle) {
                m_scatterHandle = mem.CreateScatterHandle();
                if (!m_scatterHandle)
                    return false;
            }

            for (int i = 0; i < MAX_PLAYERS; ++i) {
                const auto& p = m_players[i];

                if (!p.hObject || p.Team == m_localPlayer.Team || p.ClientID == m_localPlayer.ClientID)
                    continue;

                uintptr_t playerAddr = reinterpret_cast<uintptr_t>(p.hObject);
                mem.AddScatterReadRequest(
                    m_scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, Head),
                    &tempHeadPositions[i],
                    sizeof(D3DXVECTOR3)
                );
                mem.AddScatterReadRequest(
                    m_scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, foot),
                    &tempFootPositions[i],
                    sizeof(D3DXVECTOR3)
                );
                mem.AddScatterReadRequest(
                    m_scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, AbsolutePosition),
                    &tempAbsolutePositions[i],
                    sizeof(D3DXVECTOR3)
                );
                mem.AddScatterReadRequest(
                    m_scatterHandle,
                    reinterpret_cast<uintptr_t>(p.characFX) + offsetof(KLASSES::pCharacterFx, isDead),
                    &tempIsDeadFlags[i],
                    sizeof(int8_t)
                );
            }

            mem.ExecuteReadScatter(m_scatterHandle);

            m_headPositions = tempHeadPositions;
            m_footPositions = tempFootPositions;
            m_absolutePositions = tempAbsolutePositions;
            m_isDeadFlags = tempIsDeadFlags;

            m_minimalPlayers.clear();
            m_minimalPlayers.reserve(MAX_PLAYERS);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                const auto& p = m_players[i];
                if (!p.hObject || p.Team == m_localPlayer.Team)
                    continue;

                MinimalPlayerData mpd;
                mpd.hObject = p.hObject;
                mpd.Team = p.Team;
                mpd.Health = p.Health;
                
                memcpy(mpd.Name, p.Name, sizeof(mpd.Name));
                mpd.HeadPos = m_headPositions[i];
                mpd.FootPos = m_footPositions[i];
                mpd.AbsPos = m_absolutePositions[i];
                mpd.IsDead = (m_isDeadFlags[i] != 0);

                m_minimalPlayers.push_back(mpd);
                 
                 
            }
            return true;
        }
    };
}