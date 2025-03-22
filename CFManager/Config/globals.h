#pragma once
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

struct RGBA {
    int R, G, B, A;
};

extern RGBA g_EnemyColor;
extern RGBA g_AllyColor;
extern RGBA g_ESPLineColor;
extern RGBA g_NameColor;
extern RGBA g_HeadColor;

extern bool Dcheckbox, enableAimbot, Headcheckbox, Healthcheckbox, showInfoText;
extern bool Namecheckbox, Distancecheckbox, weaponcheckbox, Filterteams, showFPS;
extern bool crosshair_notify, Bonecheckbox, showEspLines, draw_radar;

extern float AimFov, AimSpeed, MaxAimDistance, boxtk, hptk, hdtk, bonetk;
extern int firstHotkey, esptype, fov;

extern bool Flogs[5];
extern const char* Flogss[5];


