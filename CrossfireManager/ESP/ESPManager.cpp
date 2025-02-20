#include "ESPManager.hpp"
#include <algorithm>
#include "Overlay.h"
#include "Overlay.h"
#include <vector>
#include <array>
using namespace KLASSES;
using namespace std::chrono_literals;
namespace {
    constexpr int MAX_PLAYERS = 16;
     
    
}


void DebugDrawBox(int x, int y, int w, int h) {
    Utils::DebugLog("[DEBUG] Drawing Rect at -> X: %d, Y: %d, W: %d, H: %d", x, y, w, h);
}

void ESPManager::ToggleESP(Memory& mem) {
    // Make sure your LT_SHELL or global address is valid
    if (!LT_SHELL) {
        KLASSES::Utils::DebugLog("❌ Cannot Enable ESP - LT_SHELL Not Set!");
        return;
    }

    
    // Flip from ON->OFF or OFF->ON
    const bool newState = !m_active.load(std::memory_order_relaxed);
    if (newState) {
        StartESP(mem);
    }
    else {
        StopESP();
    }

    // Debug message, or switch to your own logging
    Utils::DebugLog("[ESP] %s", newState ? "Enabled" : "Disabled");
}

 void ESPManager::StartESP(Memory& mem) {
    
    m_active.store(true, std::memory_order_relaxed);

    
    m_thread = std::jthread([this, &mem](std::stop_token stopToken) {
        ESPWorker(stopToken, mem);
        });
}

void ESPManager::StopESP() {
     
    m_active.store(false, std::memory_order_relaxed);

   
    if (m_thread.joinable()) {
        m_thread.request_stop();
        m_thread.join();
    }
}

void ESPManager::ESPWorker(std::stop_token stopToken, Memory& mem) {
     
    while (!stopToken.stop_requested() && m_active.load(std::memory_order_relaxed)) {
        DrawPlayerESP(mem);
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}

void ESPManager::DrawPlayerESP(Memory& mem) {

    static auto lastCacheUpdate = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const bool updateCache = (now - lastCacheUpdate > 64ms);

    static struct {
        LTClientShell clientShell;
        pPlayer localPlayer;
        std::vector<pPlayer> players;
        std::array<D3DXVECTOR3, MAX_PLAYERS> headPositions;
        std::array<D3DXVECTOR3, MAX_PLAYERS> footPositions;
    } cache;

    if (updateCache) {

        if (!mem.Read(LT_SHELL, &cache.clientShell, sizeof(LTClientShell))) {
            std::lock_guard<std::mutex> lock(g_espMutex);
            g_espRects.clear();
            return;
        }
        if (!cache.clientShell.CPlayerClntBase) {
            std::lock_guard<std::mutex> lock(g_espMutex);
            g_espRects.clear();
            return;
        }
        cache.localPlayer = cache.clientShell.GetLocalPlayer(mem);
        lastCacheUpdate = now;
    }

    cache.players.resize(MAX_PLAYERS);

    LT_DRAWPRIM globalDrawPrim = mem.Read<LT_DRAWPRIM>(offs::ILTDrawPrim);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        cache.players[i] = cache.clientShell.GetPlayerByIndex(i);
        if (cache.players[i].hObject &&
            cache.players[i].Team != cache.localPlayer.Team &&
            cache.players[i].ClientID != cache.localPlayer.ClientID) {
            uintptr_t playerAddr = reinterpret_cast<uintptr_t>(cache.players[i].hObject);
            cache.headPositions[i] = mem.Read<D3DXVECTOR3>(playerAddr + offsetof(obj, Head));
            cache.footPositions[i] = mem.Read<D3DXVECTOR3>(playerAddr + offsetof(obj, foot));
        }
    }

    std::vector<RectData> frameRects;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!cache.players[i].hObject ||
            cache.players[i].Team == cache.localPlayer.Team ||
            cache.players[i].ClientID == cache.localPlayer.ClientID)
            continue;

        D3DXVECTOR3 headPos = cache.headPositions[i];
        D3DXVECTOR3 footPos = cache.footPositions[i];

        if (!EngineW2S(globalDrawPrim, &headPos) ||
            !EngineW2S(globalDrawPrim, &footPos))
            continue;

        D3DXVECTOR3 bodyCenter{
            (headPos.x + footPos.x) / 2.0f,
            (headPos.y + footPos.y) / 2.0f,
            0
        };

        headPos.x = bodyCenter.x;
        footPos.x = bodyCenter.x;
        headPos.z = bodyCenter.z;
        footPos.z = bodyCenter.z;

        const float height = fabs(headPos.y - footPos.y);
        const float width = height * 0.6f;

        RectData rect;
        rect.x = static_cast<int>(bodyCenter.x - width / 2);
        rect.y = static_cast<int>(bodyCenter.y - height / 2);
        rect.w = static_cast<int>(width);
        rect.h = static_cast<int>(height);
        rect.color = ESPColorEnemy;
        rect.playerName = cache.players[i].Name;
        rect.currentHP = cache.players[i].Health;
        rect.maxHP = 100;

        frameRects.push_back(rect);
    }

    {
        std::lock_guard<std::mutex> lock(g_espMutex);
        g_espRects.swap(frameRects);
    }
}

