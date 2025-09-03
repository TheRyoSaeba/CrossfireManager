#pragma once
 
#include "../ESP/ESP.h"

extern std::atomic<WeaponType> g_lastWeapon;
extern bool g_mouseCurrentlyLocked;
 

bool HandleHotkeyMode();


void MoveCursor(float x, float y, LTClientShell _shell);
void  MoveCursor2(float targetX, float targetY, LTClientShell _shell);

bool WeaponConfig(const KLASSES::pPlayer& localPlayer, WeaponType wType);
bool StoreWeaponConfig(const KLASSES::pPlayer& localPlayer, WeaponType wType);


void outlined_text(const ImVec2& pos, ImU32 color, const char* text);


void WeaponConfigOverlay();
 
namespace Aimbot {
    struct Target;
    bool IsLockValid(const std::shared_ptr<ESP::Snapshot>& snapshot);
    Target FindBestTarget(const std::shared_ptr<ESP::Snapshot>& s, int mode);
    static D3DXVECTOR2 CalcAngle(const D3DXVECTOR3& source, const D3DXVECTOR3& target);
    static float NormalizeAngleDelta(float delta);
    static float LerpAngle(float from, float to, float factor);
    void AimAt(const D3DXVECTOR3& targetPos, float smoothingFactor, const std::shared_ptr<ESP::Snapshot>& snapshot);
    void DrawDebugInfo(const std::shared_ptr<ESP::Snapshot>& snapshot, ImDrawList* draw);
    inline bool PickClosestBone2D(const ESP::MinimalPlayerData& p,
        const ESP::Snapshot& s,
        int aimMode,
        D3DXVECTOR3& outWorld);
    void Run();
}
 