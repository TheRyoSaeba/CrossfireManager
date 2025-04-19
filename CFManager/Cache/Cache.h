#pragma once
#define NOMINMAX

#include "../region_header.h"
#include "Memory.h"
#include <array>
#include <vector>
#include <mutex>
#include <chrono>
#include <d3dx9math.h>
#include <cstddef>
#include <unordered_map>
 
constexpr size_t NUM_BONES = 19;
namespace ESP {



    struct MinimalPlayerData {
        void* hObject;
        int Team;
        int Health;
        char Name[14];
        D3DXVECTOR3 HeadPos;
        D3DXVECTOR3 FootPos;
        D3DXVECTOR3 AbsPos;
        bool IsDead;
        std::array<D3DXVECTOR3, NUM_BONES> bones;

    };


    struct Snapshot {
        D3DXVECTOR3 localAbsPos;
        D3DXVECTOR3 localHeadPos;
        bool localIsDead;
        float localYaw;
        float localPitch;
        uintptr_t local_angles;
        int localTeam;
        KLASSES::LT_DRAWPRIM drawPrim;
        KLASSES::LTClientShell m_clientShell;
        KLASSES::pPlayer LocalPlayer;
        std::vector<MinimalPlayerData> enemies;
       
    };

    class Cache {
    public:
        Cache() : m_scatterHandle(nullptr) {
            Clear();
        }

        ~Cache() {

            m_scatterHandle = nullptr;
        }

        bool Update(Memory& mem) {


            if (!ClientUpdate(mem))
                return false;


            if (!UpdateLocalPlayer2(mem)) return false;



            UpdateEntities(mem);
                


            if (!UpdatePositions(mem))
                return false;

            m_lastUpdate = std::chrono::steady_clock::now();
            return true;
        }

        Snapshot GetSnapshot() const {
            Snapshot snap;

            snap.localAbsPos = m_localAbsolutePosition;
            snap.localHeadPos = m_localHeadPosition;
            snap.localIsDead = m_localIsDead;
            snap.localYaw = m_localYaw;
            snap.localPitch = m_localPitch;
            snap.localTeam = m_localPlayer.Team;
            snap.local_angles = m_viewangles;
            snap.drawPrim = mem.Read<KLASSES::LT_DRAWPRIM>(offs::ILTDrawPrim);
            snap.m_clientShell = m_clientShell;
            snap.LocalPlayer = m_localPlayer;
            snap.enemies = m_minimalPlayers;
            return snap;
        }

        void Clear() {
            m_clientShell = KLASSES::LTClientShell{};
            m_localPlayer = KLASSES::pPlayer{};
            m_localAbsolutePosition = D3DXVECTOR3{};
            m_localHeadPosition = D3DXVECTOR3{};
            m_localIsDead = false;
            m_localYaw = 0.0f;
            m_localPitch = 0.0f;
            m_viewangles = 0;
            for (auto& buffer : m_players) {
                buffer.fill(KLASSES::pPlayer{});
            }
            m_headPositions.fill(D3DXVECTOR3{});
            m_footPositions.fill(D3DXVECTOR3{});
            m_absolutePositions.fill(D3DXVECTOR3{});
            m_isDeadFlags.fill(0);
            m_minimalPlayers.clear();
            m_lastUpdate = std::chrono::steady_clock::now();
            m_lastEntityUpdate = std::chrono::steady_clock::now();
        }

 //yes i know 

        std::chrono::steady_clock::time_point m_lastEntityUpdate;
        static constexpr std::chrono::milliseconds ENTITY_UPDATE_INTERVAL{ 200 };
        KLASSES::LTClientShell m_clientShell;
        KLASSES::pPlayer m_localPlayer;
        D3DXVECTOR3 m_localAbsolutePosition;
        D3DXVECTOR3 m_localHeadPosition;
        bool m_localIsDead;
        float m_localYaw;
        float m_localPitch;
        uintptr_t m_viewangles;
        std::array<KLASSES::pPlayer, MAX_PLAYERS> m_players[2];
        std::atomic<int> m_activeBuffer{ 0 };
        std::atomic<bool> m_entityUpdateInProgress{ false };
        uint32_t m_lastChecksum{ 0 };
        std::array<D3DXVECTOR3, MAX_PLAYERS> m_headPositions;
        std::array<D3DXVECTOR3, MAX_PLAYERS> m_footPositions;
        std::array<D3DXVECTOR3, MAX_PLAYERS> m_absolutePositions;
        std::unordered_map<int, std::unordered_map<int, D3DXVECTOR3>> m_bonePositions;
        std::array<int8_t, MAX_PLAYERS> m_isDeadFlags;
        std::vector<MinimalPlayerData> m_minimalPlayers;
        std::chrono::steady_clock::time_point m_lastUpdate;
        VMMDLL_SCATTER_HANDLE m_scatterHandle;

        bool ClientUpdate(Memory& mem) {
            if (!mem.Read(offs::LT_SHELL, &m_clientShell, sizeof(KLASSES::LTClientShell)))
                
                return false;
            return true;
        }
        bool UpdateLocalPlayer2(Memory& mem) {
            m_localPlayer = m_clientShell.GetLocalPlayer(mem);
            if (m_localPlayer.hObject != nullptr) {
                if (!m_scatterHandle) {
                    m_scatterHandle = mem.CreateScatterHandle();
                }
                uintptr_t playerAddr = reinterpret_cast<uintptr_t>(m_localPlayer.hObject);
                uintptr_t characFXAddr = reinterpret_cast<uintptr_t>(m_localPlayer.characFX);
                uintptr_t clntBaseAddr = reinterpret_cast<uintptr_t>(m_clientShell.CPlayerClntBase);
                m_viewangles = clntBaseAddr + offsetof(KLASSES::pPlayerClntBase, ViewAngles);


                mem.AddScatterReadRequest(m_scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, AbsolutePosition),
                    &m_localAbsolutePosition, sizeof(D3DXVECTOR3));


                uint32_t boneArrayPtr = 0;
                mem.AddScatterReadRequest(m_scatterHandle,
                    playerAddr + offsetof(KLASSES::obj, BoneArray),
                    &boneArrayPtr, sizeof(uint32_t));


                mem.AddScatterReadRequest(m_scatterHandle,
                    characFXAddr + offsetof(KLASSES::pCharacterFx, isDead),
                    &m_localIsDead, sizeof(bool));
                mem.AddScatterReadRequest(m_scatterHandle,
                    clntBaseAddr + offsetof(KLASSES::pPlayerClntBase, Yaw),
                    &m_localYaw, sizeof(float));
                mem.AddScatterReadRequest(m_scatterHandle,
                    clntBaseAddr + offsetof(KLASSES::pPlayerClntBase, Pitch),
                    &m_localPitch, sizeof(float));


                mem.ExecuteReadScatter(m_scatterHandle);


                if (boneArrayPtr != 0) {
                    D3DXMATRIX headBoneMatrix;

                    uintptr_t headBoneAddr = boneArrayPtr + (6 * sizeof(D3DXMATRIX));
                    mem.AddScatterReadRequest(m_scatterHandle,
                        headBoneAddr, &headBoneMatrix, sizeof(D3DXMATRIX));
                    mem.ExecuteReadScatter(m_scatterHandle);


                    m_localHeadPosition = D3DXVECTOR3(
                        headBoneMatrix._14,
                        headBoneMatrix._24,
                        headBoneMatrix._34
                    );
                }
                else {

                    m_localHeadPosition = { 0.0f, 0.0f, 0.0f };
                }
                return true;
            }
            return false;
        }
        bool UpdateLocalPlayer(Memory& mem) {



            int localIdx = mem.Read<int>((uintptr_t)offs::LT_SHELL + offs::MYOFFSET);

            uintptr_t localPlayerAddr = offs::LT_SHELL + offs::dwCPlayerStart + (localIdx * sizeof(offs::dwCPlayerSize));


            if (!m_scatterHandle) {
                m_scatterHandle = mem.CreateScatterHandle();
            }
            else {
                mem.CloseScatterHandle(m_scatterHandle);
            }


            mem.AddScatterReadRequest(m_scatterHandle, localPlayerAddr, &m_localPlayer, sizeof(offs::dwCPlayerSize));
            auto scatterStart = std::chrono::high_resolution_clock::now();
            mem.ExecuteReadScatter(m_scatterHandle);
            auto scatterEnd = std::chrono::high_resolution_clock::now();



            if (!m_localPlayer.hObject) {
                return false;
            }


            uintptr_t playerAddr = reinterpret_cast<uintptr_t>(m_localPlayer.hObject);
            uintptr_t characFXAddr = reinterpret_cast<uintptr_t>(m_localPlayer.characFX);
            uintptr_t clntBaseAddr = reinterpret_cast<uintptr_t>(m_clientShell.CPlayerClntBase);


            mem.AddScatterReadRequest(m_scatterHandle, playerAddr + offsetof(KLASSES::obj, AbsolutePosition), &m_localAbsolutePosition, sizeof(D3DXVECTOR3));
            mem.AddScatterReadRequest(m_scatterHandle, playerAddr + offsetof(KLASSES::obj, Head), &m_localHeadPosition, sizeof(D3DXVECTOR3));
            mem.AddScatterReadRequest(m_scatterHandle, characFXAddr + offsetof(KLASSES::pCharacterFx, isDead), &m_localIsDead, sizeof(bool));
            mem.AddScatterReadRequest(m_scatterHandle, clntBaseAddr + offsetof(KLASSES::pPlayerClntBase, Yaw), &m_localYaw, sizeof(float));
            mem.AddScatterReadRequest(m_scatterHandle, clntBaseAddr + offsetof(KLASSES::pPlayerClntBase, Pitch), &m_localPitch, sizeof(float));


            mem.ExecuteReadScatter(m_scatterHandle);

            return true;
        }


        /// <summary>
        /// Make sure pplayer padding matches dwcplayersize if not usn..raw
        /// </summary>
        /// <param name="mem"></param>
        void UpdateEntities(Memory& mem)
        {
            const auto now = std::chrono::steady_clock::now();
            if ((now - m_lastEntityUpdate < ENTITY_UPDATE_INTERVAL) ||
                m_entityUpdateInProgress.load()) {
                return;
            }

            m_entityUpdateInProgress.store(true);
            m_lastEntityUpdate = now;

            const auto startTime = std::chrono::steady_clock::now();

            const int inactiveBuffer = 1 - m_activeBuffer.load();
            auto& targetBuffer = m_players[inactiveBuffer];

            const uintptr_t ENTITY_BASE = offs::LT_SHELL + offs::dwCPlayerStart;
            const size_t entrySize = offs::dwCPlayerSize;
            const size_t totalSize = entrySize * MAX_PLAYERS;

           
            std::vector<std::byte> rawBuffer(totalSize);

            bool bulkSuccess = mem.Read(ENTITY_BASE, rawBuffer.data(), totalSize);

            if (bulkSuccess)
            {
               
                for (int i = 0; i < MAX_PLAYERS; ++i)
                {
                    std::byte* src = rawBuffer.data() + (i * entrySize);
                    memcpy(&targetBuffer[i], src, std::min(sizeof(KLASSES::pPlayer), entrySize));
                }

                m_activeBuffer.store(inactiveBuffer);
               // LOG("[UpdateEntities] Bulk read and decode successful. Swapped buffers.\n");
            }
            else
            {
                //OG("[UpdateEntities] Bulk read failed! Falling back to GetPlayerByIndex.\n");

                for (int i = 0; i < MAX_PLAYERS; ++i)
                {
                    targetBuffer[i] = m_clientShell.GetPlayerByIndex(i);
                }

                m_activeBuffer.store(inactiveBuffer);
                LOG("[UpdateEntities] Fallback update complete. Swapped buffers.\n");
            }

            const auto endTime = std::chrono::steady_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            //LOG("[UpdateEntities] Time taken: %lld ms\n", duration.count());

            m_entityUpdateInProgress.store(false);
        }




        bool UpdatePositions(Memory& mem) {
            std::array<D3DXVECTOR3, MAX_PLAYERS> tempHeadPositions;
            std::array<D3DXVECTOR3, MAX_PLAYERS> tempFootPositions;
            std::array<D3DXVECTOR3, MAX_PLAYERS> tempAbsolutePositions;
            std::array<int8_t, MAX_PLAYERS> tempIsDeadFlags{};
            std::array<uintptr_t, MAX_PLAYERS> tempBoneArrayAddresses{};

            std::array<D3DXMATRIX, MAX_PLAYERS> tempHeadBoneMatrices{};


            std::array<std::array<D3DXVECTOR3, NUM_BONES>, MAX_PLAYERS> tempExtraBones;

            const int activeBuffer = m_activeBuffer.load();
            const auto& currentPlayers = m_players[activeBuffer];


            if (!m_scatterHandle) {
                m_scatterHandle = mem.CreateScatterHandle();
            }
            else {
                mem.CloseScatterHandle(m_scatterHandle);
                m_scatterHandle = mem.CreateScatterHandle();
            }


            for (int i = 0; i < MAX_PLAYERS; ++i) {
                const auto& p = currentPlayers[i];
                if (!p.hObject || p.ClientID == m_localPlayer.ClientID)
                    continue;

                uintptr_t playerAddr = reinterpret_cast<uintptr_t>(p.hObject);
                uintptr_t boneArrayPtrAddr = playerAddr + offsetof(KLASSES::obj, BoneArray);

                mem.AddScatterReadRequest(m_scatterHandle, boneArrayPtrAddr, &tempBoneArrayAddresses[i], sizeof(uintptr_t));
                mem.AddScatterReadRequest(m_scatterHandle, playerAddr + offsetof(KLASSES::obj, foot), &tempFootPositions[i], sizeof(D3DXVECTOR3));
                mem.AddScatterReadRequest(m_scatterHandle, playerAddr + offsetof(KLASSES::obj, AbsolutePosition), &tempAbsolutePositions[i], sizeof(D3DXVECTOR3));
                mem.AddScatterReadRequest(m_scatterHandle, reinterpret_cast<uintptr_t>(p.characFX) + offsetof(KLASSES::pCharacterFx, isDead), &tempIsDeadFlags[i], sizeof(int8_t));
            }
            mem.ExecuteReadScatter(m_scatterHandle);

            
            bool needBones = (Bonecheckbox || AimPosition  == 1 || AimPosition == 2);
            if (needBones) {

                std::vector<int> boneGroups = {
                    6, 5, 4, 3, 1,
                    21, 22, 23, 25, 26,
                    27, 14, 15, 16, 17,
                    7, 8, 9,
                    10,
                };


                for (int i = 0; i < MAX_PLAYERS; ++i) {
                    if (!tempBoneArrayAddresses[i])
                        continue;


                    std::array<D3DXMATRIX, NUM_BONES> localMats;

                    for (size_t j = 0; j < NUM_BONES; j++) {
                        int bone = boneGroups[j];
                        uintptr_t matrixAddr = tempBoneArrayAddresses[i] + (bone * sizeof(D3DXMATRIX));
                        mem.AddScatterReadRequest(m_scatterHandle, matrixAddr, &localMats[j], sizeof(D3DXMATRIX));
                    }
                    mem.ExecuteReadScatter(m_scatterHandle);


                    std::array<D3DXVECTOR3, NUM_BONES> bonePositions;
                    for (size_t j = 0; j < NUM_BONES; j++) {
                        bonePositions[j].x = localMats[j]._14;
                        bonePositions[j].y = localMats[j]._24;
                        bonePositions[j].z = localMats[j]._34;
                    }
                    tempExtraBones[i] = bonePositions;
                }

                for (int i = 0; i < MAX_PLAYERS; ++i) {
                    if (!tempBoneArrayAddresses[i])
                        continue;
                    tempHeadPositions[i] = tempExtraBones[i][0];   
                }
            }
            else {

                for (int i = 0; i < MAX_PLAYERS; ++i) {
                    if (!tempBoneArrayAddresses[i])
                        continue;
                    uintptr_t headBoneAddr = tempBoneArrayAddresses[i] + (6 * sizeof(D3DXMATRIX));
                    mem.AddScatterReadRequest(m_scatterHandle, headBoneAddr, &tempHeadBoneMatrices[i], sizeof(D3DXMATRIX));
                    mem.ExecuteReadScatter(m_scatterHandle);
                }
                for (int j = 0; j < MAX_PLAYERS; ++j) {
                    tempHeadPositions[j] = D3DXVECTOR3(
                        tempHeadBoneMatrices[j]._14,
                        tempHeadBoneMatrices[j]._24,
                        tempHeadBoneMatrices[j]._34
                    );
                }
            }


            m_headPositions = tempHeadPositions;
            m_footPositions = tempFootPositions;
            m_absolutePositions = tempAbsolutePositions;
            m_isDeadFlags = tempIsDeadFlags;

            m_minimalPlayers.clear();
            m_minimalPlayers.reserve(MAX_PLAYERS);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                const auto& p = currentPlayers[i];
                if (!p.hObject || p.ClientID == m_localPlayer.ClientID)
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
                if (needBones) {

                    mpd.bones = tempExtraBones[i];
                }
                m_minimalPlayers.push_back(mpd);
            }

            return true;
        }
    };
}