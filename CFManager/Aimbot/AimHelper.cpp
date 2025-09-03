
#include "AimHelper.h"
#include <window.h>
#include "KMBOX.h"
#include <random>
#include <chrono>
#include <windows.h>
#include <d3dx9math.h>
std::atomic<WeaponType> g_lastWeapon{ static_cast<WeaponType>(-1) };
bool g_mouseCurrentlyLocked = false;



static bool HandleHotkeyMode()
{
    static bool isAimbotEnabled = false;
    static bool wasPressed = false;

    bool isPressed =
        (GetAsyncKeyState(aimkey) & 0x8000) ||
        mem.GetKeyboard()->IsKeyDown(aimkey);

    switch (keymode)
    {
    case 0:  
    {
        if (isPressed && !wasPressed)
        {
            isAimbotEnabled = !isAimbotEnabled;
          
        }
        wasPressed = isPressed;
        return isAimbotEnabled;
    }

    case 1: 
    {
        if (isPressed && !wasPressed)
        {
           
        }
        wasPressed = isPressed;
        return isPressed;
    }

    default:
        return false;
    }
}

void outlined_text(const ImVec2& pos, ImU32 color, const char* text) {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImFont* font = ImGui::GetFont();
    float       size = ImGui::GetFontSize() * 4.0f;
    static const ImU32 back = ImColor(1, 1, 1);
    draw->AddText({ pos.x + 1, pos.y }, back, text);
    draw->AddText({ pos.x - 1, pos.y }, back, text);
    draw->AddText({ pos.x, pos.y + 1 }, back, text);
    draw->AddText({ pos.x, pos.y - 1 }, back, text);
    draw->AddText(pos, color, text);
}

 
 

void MoveCursor(float targetX, float targetY, LTClientShell _shell)
{
    static float rx = 0.f, ry = 0.f;
    float curFov = KLASSES::PlayerViewManager::GetZoomLevel(mem, _shell);
    if (curFov < 0.f)  curFov = FOV_DEFAULT;

    float fovRatio = std::clamp(curFov / FOV_DEFAULT, 0.5f, 1.0f);
    float dx = targetX * fovRatio;
    float dy = targetY * fovRatio;

    float mag = std::sqrt(dx * dx + dy * dy) + 1e-6f;
    float norm = std::min(mag / 200.f, 1.f);
    float baseSmooth = std::clamp(smoothing * 3.0f, 1.f, 15.f);
    float jitter = ((rand() % 101) - 50) / 1000.f;
    float useSmooth = std::max(1.f, baseSmooth + jitter + norm * 2.0f);

    float sx = dx / useSmooth;
    float sy = dy / useSmooth;

    const float kMax = 12.f;
    sx = std::clamp(sx, -kMax, kMax);
    sy = std::clamp(sy, -kMax, kMax);

    rx += sx; ry += sy;
    int mx = (int)std::lrint(rx);  rx -= mx;
    int my = (int)std::lrint(ry);  ry -= my;

    if (mx == 0 && my == 0) return;

    kmBoxBMgr.km_move(mx, my);
    KmBoxNETMgr.Mouse.Move(mx, my);
    
}

void MoveCursor2(float targetX, float targetY, LTClientShell _shell)
{
    static float residualX = 0.0f;
    static float residualY = 0.0f;

    float currentFOV = KLASSES::PlayerViewManager::GetZoomLevel(mem, _shell);
    if (currentFOV < 0) currentFOV = FOV_DEFAULT;

    float fovRatio = std::clamp(currentFOV / FOV_DEFAULT, 0.50f, 1.0f);
    float zoomMultiplier = std::clamp(FOV_DEFAULT / currentFOV, 1.0f, 10.0f);

    int stepCount = std::clamp(int(smoothing * 2 * zoomMultiplier), 1, 15);
    float moveX = (targetX / static_cast<float>(stepCount)) * fovRatio + residualX;
    float moveY = (targetY / static_cast<float>(stepCount)) * fovRatio + residualY;

    float length = sqrtf(moveX * moveX + moveY * moveY);
    float maxMove = std::clamp(length, 1.0f, 5.0f);

    if (length > maxMove) {
        float scale = maxMove / length;
        moveX *= scale;
        moveY *= scale;
    }

    float deadzone = 1.0f * fovRatio;
    int finalMoveX = 0, finalMoveY = 0;

    if (fabsf(moveX) >= deadzone) {
        finalMoveX = static_cast<int>(roundf(moveX));
        residualX = moveX - finalMoveX;
    }
    else {
        residualX += moveX;
    }

    if (fabsf(moveY) >= deadzone) {
        finalMoveY = static_cast<int>(roundf(moveY));
        residualY = moveY - finalMoveY;
    }
    else {
        residualY += moveY;
    }

    residualX = std::clamp(residualX, -1.0f, 1.0f);
    residualY = std::clamp(residualY, -1.0f, 1.0f);



    if (finalMoveX != 0 || finalMoveY != 0) {
        kmBoxBMgr.lock_mx();
        kmBoxBMgr.lock_my();
        kmBoxBMgr.km_move(finalMoveX, finalMoveY);
        kmBoxBMgr.unlock_my();
        kmBoxBMgr.unlock_mx();
    }
}



bool WeaponConfig(const KLASSES::pPlayer& localPlayer, WeaponType wType)
{
    auto it = g_weaponAimbotConfigs.find(wType);
    if (it == g_weaponAimbotConfigs.end())
        return false;

    const auto& cfg = it->second;

    AimFov = cfg.AimFov;
    MemoryMode = cfg.MemoryMode;
    ToggleorHold = cfg.ToggleorHold;
    aimkey = cfg.aimkey;
    keymode = cfg.keymode;
    smoothing = cfg.smoothing;
    selectedAimDevice = cfg.selectedAimDevice;
    AimPosition = cfg.AimPosition;
    TargetSwitch = cfg.TargetSwitch;
    MaxAimDistance = cfg.MaxAimDistance;
    firstHotkey = cfg.firstHotkey;

    return true;
}

bool StoreWeaponConfig(const KLASSES::pPlayer& localPlayer, WeaponType wType)
{

    auto it = g_weaponAimbotConfigs.find(wType);
    if (it == g_weaponAimbotConfigs.end())
        return false;


    auto& cfg = it->second;
    cfg.AimFov = AimFov;
    cfg.MemoryMode = MemoryMode;
    cfg.ToggleorHold = ToggleorHold;
    cfg.aimkey = aimkey;
    cfg.keymode = keymode;
    cfg.smoothing = smoothing;
    cfg.selectedAimDevice = selectedAimDevice;
    cfg.AimPosition = AimPosition;
    cfg.TargetSwitch = TargetSwitch;
    cfg.MaxAimDistance = MaxAimDistance;
    cfg.firstHotkey = firstHotkey;

    return true;
}

void WeaponConfigOverlay()
{
    WeaponType w = g_lastWeapon.load(std::memory_order_relaxed);
    static const char* names[] =
    { "Pistol","Shotgun","SMG","Rifle","Sniper","Machine Gun",
      "Grenades","Knife","","C4","Rappel" };

    int idx = (int)w;
    const char* cls = (idx >= 0 && idx < 11) ? names[idx] : "Unknown";
    char buf[64];
    snprintf(buf, sizeof(buf), xorstr_("Current class: %s"), cls);


    outlined_text(ImVec2(10, 10), IM_COL32(0, 255, 0, 255), buf);
}


namespace Aimbot {
    struct Target {
        const ESP::MinimalPlayerData* player = nullptr;
        D3DXVECTOR3                   position{};
        float                         screenDistance = FLT_MAX;
    };

        Target FindBestTarget(const std::shared_ptr<ESP::Snapshot>&s, int aimMode)
        {
            Target best{};
            if (!s) return best;

            const auto& d = s->drawPrim;
            const float fovPx = AimFov * (d.viewport.Height / 180.0f);

            float bestScore = FLT_MAX;
            for (const auto& p : s->enemies) {
                if (p.IsDead || p.Team == s->localTeam) continue;


                const float maxWorld = MaxAimDistance * 100.0f;
                D3DXVECTOR3 toP = p.AbsPos - s->localAbsPos;
                if (D3DXVec3Length(&toP) > maxWorld) continue;

                D3DXVECTOR3 boneWorld{};
                if (!PickClosestBone2D(p, *s, aimMode, boneWorld)) continue;


                D3DXVECTOR3 sp = boneWorld;
                if (!EngineW2S(d, &sp)) continue;


                const float cx = d.viewport.X + d.viewport.Width * 0.5f;
                const float cy = d.viewport.Y + d.viewport.Height * 0.5f;
                const float dx = sp.x - cx, dy = sp.y - cy;
                const float screenDist = std::sqrt(dx * dx + dy * dy);
                if (screenDist > fovPx) continue;

                const float normWorld = D3DXVec3Length(&toP) / maxWorld;
                const float normScreen = screenDist / fovPx;
                float score = (TargetSwitch == 1) ? normWorld
                    : (TargetSwitch == 2) ? (normWorld * 0.6f + normScreen * 0.4f)
                    : normScreen;

                if (score < bestScore) {
                    bestScore = score;
                    best.player = &p;
                    best.position = boneWorld;
                    best.screenDistance = screenDist;
                }
            }
            return best;
        }

 

    void Run()
    {
        auto s = g_cacheManager.GetSnapshot();
        if (!s) return;

        if (perWeaponConfig)
        {
            auto local = s->LocalPlayer;
            WeaponType cur = CWeapon::GetCurrentWeaponType(local);
            WeaponType prev = g_lastWeapon.load(std::memory_order_relaxed);
            if (cur != prev)
            {
                if (prev != static_cast<WeaponType>(-1))
                    StoreWeaponConfig(local, prev);
                WeaponConfig(local, cur);
                g_lastWeapon.store(cur, std::memory_order_relaxed);
            }
        }

        if (!s->m_clientShell.inGame() || !HandleHotkeyMode())
            return;

       
        Target tgt = FindBestTarget(s, AimPosition);
        if (!tgt.player)
            return;
        D3DXVECTOR3 sp = tgt.position;
        if (!EngineW2S(s->drawPrim, &sp))
            return;

        auto& vp = s->drawPrim.viewport;
        float cx = vp.X + vp.Width * 0.5f;
        float cy = vp.Y + vp.Height * 0.5f;
        float dx = sp.x - cx;
        float dy = sp.y - cy - (vp.Height * 0.010f);

        if (fabsf(dx) < 0.5f && fabsf(dy) < 0.5f)
            return;
        if (kmbox_connected)
        {
            if (selectedAimDevice == 0)
                MoveCursor(dx, dy, s->m_clientShell);

            if (selectedAimDevice == 1)
            {
                MoveCursor2(dx, dy, s->m_clientShell);
            }
        }
       
    }
    
    static constexpr int kClosestOrder[] = {
        2, 3, 4,   
        1, 0,       
        16, 18  
    };

    inline bool PickClosestBone2D(const ESP::MinimalPlayerData& p,
        const ESP::Snapshot& s,
        int aimMode,           
        D3DXVECTOR3& outWorld)
    {
        const auto& d = s.drawPrim;
        const float fovPx = AimFov * (d.viewport.Height / 180.0f);
        const ImVec2 center{
            d.viewport.X + d.viewport.Width * 0.5f,
            d.viewport.Y + d.viewport.Height * 0.5f
        };

       
        if (aimMode == 0) { outWorld = p.HeadPos; return true; }
        if (aimMode == 1 && p.bones.size() > 3) { outWorld = p.bones[2]; return true; }


        float bestScore = FLT_MAX;
        D3DXVECTOR3 best{};
        bool found = false;

        const int nb = static_cast<int>(p.bones.size());
        for (int bi : kClosestOrder) {
            if (bi < 0 || bi >= nb) continue;

            D3DXVECTOR3 world = p.bones[bi];
            D3DXVECTOR3 scr = world;
            if (!EngineW2S(d, &scr)) continue;

            const float dx = scr.x - center.x;
            const float dy = scr.y - center.y;
            const float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > fovPx) continue;

            float weight = 1.0f;
            if (bi == 0) weight = 1.10f;     
            else if (bi == 2 || bi == 3 || bi == 4) weight = 0.95f; 

            const float score = dist * weight;
            if (score < bestScore) {
                bestScore = score;
                best = world;
                found = true;
            }
        }

        if (found) outWorld = best;
        return found;
    }
 

}


