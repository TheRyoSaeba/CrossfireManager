
#include "AimHelper.h"
#include <window.h>
#include "KMBOX.h"
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
            char buf[64];
            snprintf(buf, sizeof(buf),
                "[Hotkey] Toggled Aimbot: %s",
                isAimbotEnabled ? "ON" : "OFF");
            outlined_text(
                ImVec2(10.f, 10.f),
                IM_COL32(0, 255, 0, 255),
                buf
            );
        }
        wasPressed = isPressed;
        return isAimbotEnabled;
    }

    case 1: 
    {
        if (isPressed && !wasPressed)
        {
            outlined_text(
                ImVec2(10.f, 10.f),
                IM_COL32(0, 255, 255, 255),
                "[Hotkey] Aimbot Key Held"
            );
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
    static float residualX = 0.0f;
    static float residualY = 0.0f;

    float currentFOV = KLASSES::PlayerViewManager::GetZoomLevel(mem, _shell);
    if (currentFOV < 0) currentFOV = FOV_DEFAULT;

    float fovRatio = std::clamp(currentFOV / FOV_DEFAULT, 0.10f, 1.0f);
    float zoomMultiplier = std::clamp(FOV_DEFAULT / currentFOV, 1.0f, 10.0f);

    int stepCount = std::clamp(int(smoothing * 2 * zoomMultiplier), 1, 20);
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
    AimSpeed = cfg.AimSpeed;
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
    snprintf(buf, sizeof(buf), "Current class: %s", cls);


    outlined_text(ImVec2(10, 10), IM_COL32(0, 255, 0, 255), buf);
}


namespace Aimbot {
    struct Target {
        const ESP::MinimalPlayerData* player = nullptr;
        D3DXVECTOR3                   position{};
        float                         screenDistance = FLT_MAX;
    };

    Target FindBestTarget(const std::shared_ptr<ESP::Snapshot>& s, int m)
    {
        Target best{};
        if (!s) return best;

        const auto& d = s->drawPrim;
        ImVec2 center{
            d.viewport.X + d.viewport.Width * 0.5f,
            d.viewport.Y + d.viewport.Height * 0.5f
        };
        float fovPx = AimFov * (d.viewport.Height / 180.0f);
        float maxWorld = MaxAimDistance * 100.0f;

        float bestScore = FLT_MAX;
        const int ib[] = { 0, 1, 2, 3, 16, 18 };

        for (const auto& p : s->enemies)
        {
            if (p.IsDead || p.Team == s->localTeam) continue;

            D3DXVECTOR3 toP = p.AbsPos - s->localAbsPos;
            float worldDist = D3DXVec3Length(&toP);
            if (worldDist > maxWorld) continue;

            D3DXVECTOR3 aimPoint;
            if (m == 0)
            {
                aimPoint = p.HeadPos;
            }
            else if (m == 1 && p.bones.size() > 2)
            {
                aimPoint = p.bones[2];
            }
            else if (m == 2)
            {
                float boneBest = FLT_MAX;
                aimPoint = p.HeadPos;
                for (int bi : ib)
                {
                    if (bi < (int)p.bones.size())
                    {
                        D3DXVECTOR3 bpos = p.bones[bi];
                        D3DXVECTOR3 sp = bpos;
                        if (!EngineW2S(d, &sp)) continue;
                        float dx = sp.x - center.x;
                        float dy = sp.y - center.y;
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist < boneBest)
                        {
                            boneBest = dist;
                            aimPoint = bpos;
                        }
                    }
                }
            }
            else
            {
                aimPoint = p.HeadPos;
            }

            D3DXVECTOR3 sp = aimPoint;
            if (!EngineW2S(d, &sp)) continue;

            float dx = sp.x - center.x;
            float dy = sp.y - center.y;
            float screenDist = sqrtf(dx * dx + dy * dy);
            if (screenDist > fovPx) continue;

            float normWorldDist = worldDist / maxWorld;
            float normScreenDist = screenDist / fovPx;

            float score;
            switch (TargetSwitch) {
            case 0: 
                score = normScreenDist;
                break;
            case 1: 
                score = normWorldDist;
                break;
            case 2: 
                score = normWorldDist * 0.6f + normScreenDist * 0.4f;  //bias
                break;
            default:
                score = normScreenDist;
            }

            if (score < bestScore)
            {
                best.player = &p;
                best.position = aimPoint;
                best.screenDistance = screenDist;
                bestScore = score;
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
            MoveCursor(dx, dy, s->m_clientShell);
    }

    


}
