#pragma once
 
#include "../ESP/ESP.h"

extern std::atomic<WeaponType> g_lastWeapon;
extern bool g_mouseCurrentlyLocked;
 

bool HandleHotkeyMode();


void MoveCursor(float x, float y, LTClientShell _shell);


bool WeaponConfig(const KLASSES::pPlayer& localPlayer, WeaponType wType);
bool StoreWeaponConfig(const KLASSES::pPlayer& localPlayer, WeaponType wType);


void outlined_text(const ImVec2& pos, ImU32 color, const char* text);


void WeaponConfigOverlay();

namespace Aimbot {
    struct Target;
    bool IsLockValid(const std::shared_ptr<ESP::Snapshot>& snapshot);
    Target FindBestTarget(const std::shared_ptr<ESP::Snapshot>& s, int mode);
    void DrawDebugInfo(const std::shared_ptr<ESP::Snapshot>& snapshot, ImDrawList* draw);
    void Run();
}
 