#pragma once
 
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include  <map>
#include <d3dx9.h>
 
struct RGBA {
    int R, G, B, A;
};

enum class WeaponType : int
{
    Pistol = 0,
    Shotgun = 1,
    SMG = 2,
    Rifle = 3,
    Sniper = 4,
    MachineGun = 5,
    Grenades = 6,
    Knife = 7,
    C4 = 9,
    RAPPEL = 10

};
struct WeaponAimbotConfig
{
    bool  enableAimbot = true;
    float AimFov = 25.f;
    int   MemoryMode = 0;
    int   ToggleorHold = 0;
    int    smoothing = 5;
    int    keymode = 0;
    int    aimkey = 5;
    float  AimSpeed = 0.6f;
    int   selectedAimDevice = 0;
    int   AimPosition = 0;
    int   TargetSwitch = 0;
    float MaxAimDistance = 180.f;
    int   firstHotkey = 0;
};
 
extern RGBA g_EnemyColor;
extern RGBA g_AllyColor;
extern RGBA g_ESPLineColor;
extern RGBA g_NameColor;
extern RGBA g_HeadColor;
extern bool kmbox_connected;
extern bool  memwrite, fast_knives, camera_hacks, super_kill,Dcheckbox, draw_enemies_as_arrows, draw_enemy_names, enableAimbot, perWeaponConfig, Headcheckbox, Healthcheckbox, showInfoText;
extern bool Namecheckbox, Distancecheckbox, weaponcheckbox, Filterteams, showFPS;
extern bool crosshair_notify, Bonecheckbox, showEspLines, showFOVCircle, draw_radar;
extern float  color[4], radar_size_factor, radar_elev_threshold, radar_bg_alpha, radarRadius, radarMaxDistance, radar_opacity, radar_zoom, AimFov, AimSpeed,   MaxAimDistance, boxtk, hptk, hdtk,  bonetk;
extern int  smoothing,showhidekey, aimkey, keymode, firstHotkey, esptype, fov, TargetSwitch, selectedAimDevice, ToggleorHold, AimPosition, MemoryMode, selectedPerspective;
extern D3DXVECTOR3  camOffset;
extern bool Flogs[5];
extern const char* Flogss[5];
inline std::map<WeaponType, WeaponAimbotConfig> g_weaponAimbotConfigs;
 
 