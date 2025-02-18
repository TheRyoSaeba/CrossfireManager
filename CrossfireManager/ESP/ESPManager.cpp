#include "ESPManager.hpp"
#include <algorithm>
#include "Overlay.h"
#include "Overlay.h"

using namespace KLASSES;



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
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void ESPManager::DrawPlayerESP(Memory& mem) {
    LTClientShell clientShell = mem.Read<LTClientShell>(LT_SHELL);
    if (!LT_SHELL) return;

    if (!clientShell.CPlayerClntBase) {
        std::lock_guard<std::mutex> lock(g_espMutex);
        g_espRects.clear();
        return;
    }

    // Acquire local player for enemy filtering
    pPlayer localPlayer = clientShell.GetLocalPlayer(mem);
    if (!localPlayer.hObject) return;

    constexpr int MAX_PLAYERS = 24;
    int validPlayers = 0;

    // Prepare arrays for scatter reading.
    std::vector<pPlayer> players(MAX_PLAYERS);
    std::vector<pCharacterFx> charFxArr(MAX_PLAYERS);
    std::vector<D3DXVECTOR3> headArr(MAX_PLAYERS);
    std::vector<D3DXVECTOR3> footArr(MAX_PLAYERS);

    // Gather players from the client shell
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = clientShell.GetPlayerByIndex(i);
    }

    // Create a scatter handle to batch our memory reads.
    VMMDLL_SCATTER_HANDLE handle = mem.CreateScatterHandle();

    // Add scatter read requests for each valid enemy player.
    for (int i = 0; i < MAX_PLAYERS; i++) {
       
        pPlayer player = players[i];
        // Filter: skip if invalid, same team, or same client
        if (!player.hObject || player.Team == localPlayer.Team || player.ClientID == localPlayer.ClientID)
            continue;

        // Request the character FX structure.
        mem.AddScatterReadRequest(handle, (uintptr_t)player.characFX, &charFxArr[i], sizeof(pCharacterFx));

        // Use the same offsets as in GetHead/GetFoot:
        mem.AddScatterReadRequest(handle, (uintptr_t)player.hObject + offsetof(obj, Head), &headArr[i], sizeof(D3DXVECTOR3));
        mem.AddScatterReadRequest(handle, (uintptr_t)player.hObject + offsetof(obj, foot), &footArr[i], sizeof(D3DXVECTOR3));
    }

    // Execute the batched scatter read requests.
    mem.ExecuteReadScatter(handle);

    // Process the scatter read results and add ESP rectangle data.
    for (int i = 0; i < MAX_PLAYERS; i++) {
        pPlayer player = players[i];
        if (!player.hObject || player.Team == localPlayer.Team || player.Name == localPlayer.Name)
            continue;

        // Get the character FX result.
        pCharacterFx characterFx = charFxArr[i];
       

        // Get head and foot positions from our scatter results.
        D3DXVECTOR3 headPos = headArr[i];
        D3DXVECTOR3 footPos = footArr[i];

        if (!World2Screen(&headPos) || !World2Screen(&footPos))
            continue;

        float height = std::abs(footPos.y - headPos.y);
        float width = height / 2.0f;
        if (headPos.z > 500.0f)
            continue;

        Utils::DebugLog("📌 Enemy Head -> X: %.2f, Y: %.2f, Z: %.2f", headPos.x, headPos.y, headPos.z);

        RectData rect;
        rect.x = static_cast<int>(headPos.x - width / 2);
        rect.y = static_cast<int>(headPos.y);
        rect.w = static_cast<int>(width);
        rect.h = static_cast<int>(height);
        rect.color = ESPColorEnemy;  // {255, 0, 0, 255}
        rect.playerName = player.Name;
        rect.currentHP = player.Health;
        rect.maxHP = 100;

        {
            std::lock_guard<std::mutex> lock(g_espMutex);
            g_espRects.push_back(rect);
        }
        validPlayers++;
    }

    Utils::DebugLog("[ESP] Total players drawn: %d", validPlayers);
}

