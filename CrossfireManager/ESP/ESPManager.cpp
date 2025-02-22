#include "ESPManager.hpp"
#include <algorithm>
#include "Overlay.h"
#include <vector>
#include <array>
#include <mutex>
#include "Cache.h"
#include "CacheManager.h"
using namespace KLASSES;
using namespace std::chrono_literals;
std::mutex g_espRectsMutex;
std::vector<RectData> g_espRectsFront;
 
namespace {
    constexpr int MAX_PLAYERS = 16;

    

  
}



void ESPManager::ToggleESP(Memory& mem) {
   
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

    g_cacheManager.StartUpdateThread(mem);

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
        
        auto snapshot = g_cacheManager.GetSnapshot();
        if (!snapshot || snapshot->enemies.empty()) {
           
            {
                std::lock_guard<std::mutex> lock(g_espRectsMutex);
                g_espRectsFront.clear();
            }
            std::this_thread::sleep_for(8ms);
            continue;
        }

        DrawPlayerESP(mem, snapshot);

        std::this_thread::sleep_for(8ms);
    }
}


void ESPManager::DrawPlayerESP(Memory& mem, const std::shared_ptr<ESP::Snapshot>& snapshot) {
    std::vector<RectData> frameRects;

    LT_DRAWPRIM globalDrawPrim = mem.Read<LT_DRAWPRIM>(offs::ILTDrawPrim);

    D3DXVECTOR3 localAbsPos = snapshot->localAbsPos;

    for (const auto& enemy : snapshot->enemies) {

        D3DXVECTOR3 headPos = enemy.HeadPos;
        D3DXVECTOR3 footPos = enemy.FootPos;
        if (!EngineW2S(globalDrawPrim, &headPos) || !EngineW2S(globalDrawPrim, &footPos))
            continue;

        D3DXVECTOR3 bodyCenter = {
            (headPos.x + footPos.x) / 2.0f,
            (headPos.y + footPos.y) / 2.0f,
            0.0f
        };
        float height = fabs(headPos.y - footPos.y);
        float width = height * 0.6f;

        float dx = enemy.AbsPos.x - localAbsPos.x;
        float dy = enemy.AbsPos.y - localAbsPos.y;
        float dz = enemy.AbsPos.z - localAbsPos.z;
        float distanceMeters = std::max((sqrtf(dx * dx + dy * dy + dz * dz) - 250.0f) / 100.0f, 0.0f);

        char distanceText[32];
        sprintf(distanceText, " [%.1fm]", distanceMeters);

        std::string nameWithDistance = std::string(enemy.Name, strnlen(enemy.Name, sizeof(enemy.Name))) + distanceText;

        RectData rect;
        rect.x = static_cast<int>(bodyCenter.x - width / 2);
        rect.y = static_cast<int>(bodyCenter.y - height / 2);
        rect.w = static_cast<int>(width);
        rect.h = static_cast<int>(height);
        rect.color = ESPColorEnemy;
        rect.playerName = nameWithDistance;
        rect.currentHP = enemy.Health;
        rect.maxHP = 100;

        frameRects.push_back(rect);
    }

    {
        std::lock_guard<std::mutex> lock(g_espRectsMutex);
        g_espRectsFront.swap(frameRects);
    }
}

  /*void RETURNESP(Memory& mem, bool enable, BYTE R, BYTE G, BYTE B)
{

    {
        std::lock_guard<std::mutex> lock(g_espMutex);
        if (!ESP::gCache.ClientUpdate(mem, LT_SHELL)) {
            g_espRects.clear();
            return;
        }
        ESP::gCache.UpdateLocalPlayer(mem);
        ESP::gCache.EntitiesUpdate(mem);
        ESP::gCache.PositionUpdate(mem);
        ESP::gCache.UpdateTimestamp();
    }

    uintptr_t codeCave = 0;
    auto moduleList = mem.GetModuleList("crossfire.exe");

    for (const auto& mod : moduleList) {
        if (mod.find(".dll") != std::string::npos) {
            codeCave = mem.shellcode.find_codecave(512, "crossfire.exe", mod.c_str());
            if (codeCave) {
            Utils::DebugLog("[+] Found executable code cave in %s at 0x%llX", mod.c_str(), codeCave);
                break;
            }
        }
    }

    if (!codeCave) {
        Utils::DebugLog("[!] No executable code caves found in any module!");
        return;
    }

    uintptr_t paramAddr = codeCave;
    uintptr_t stubAddr = codeCave + sizeof(uintptr_t) * ESP::MAX_PLAYERS;

    BYTE stub[] = {
        0x48, 0x83, 0xEC, 0x28,
        0x48, 0xBB,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x48, 0x8B, 0x4B, 0x08,
        0x48, 0x8B, 0x03,
        0x48, 0xFF, 0xD0,
        0x48, 0x83, 0xC4, 0x28,
        0xC3
    };

    *(uintptr_t*)&stub[6] = paramAddr;
    mem.Write(stubAddr, stub, sizeof(stub));

    struct GlowParams {
        uintptr_t funcPtr;
        uintptr_t objectPtr;
        bool enable;
        BYTE r, g, b;
    };

    std::vector<GlowParams> params;
    params.reserve(ESP::MAX_PLAYERS);

    uintptr_t functionPtr = 0x14003E5A0;

    for (int i = 0; i < ESP::MAX_PLAYERS; i++) {
        auto& player = ESP::gCache.players[i];

        if (!player.hObject || player.Team == ESP::gCache.localPlayer.Team || player.ClientID == ESP::gCache.localPlayer.ClientID)
            continue;

        uintptr_t playerObjectPtr = (uintptr_t)player.hObject;
        bool shouldEnableGlow = enable;
        BYTE redComponent = R;
        BYTE greenComponent = G;
        BYTE blueComponent = B;

        Utils::DebugLog("[+] Adding Player %d to Glow List -> Obj: 0x%llX | Enable: %d | RGB(%d, %d, %d)",
            i, playerObjectPtr, shouldEnableGlow, redComponent, greenComponent, blueComponent);

        params.push_back({
            functionPtr,
            playerObjectPtr,
            shouldEnableGlow,
            redComponent,
            greenComponent,
            blueComponent
            });
    }

    if (params.empty()) {
        Utils::DebugLog("[!] No valid enemies found for Glow ESP.");
        return;
    }

    mem.Write(paramAddr, params.data(), params.size() * sizeof(GlowParams));

    if (mem.OverwriteReturnAddress(stubAddr)) {
        Utils::DebugLog("[+] HookGlowESPS: Successfully triggered execution!");
    }
    else {
        Utils::DebugLog("[!] HookGlowESPS: Failed to hijack return address.");
    }
}*/ 


