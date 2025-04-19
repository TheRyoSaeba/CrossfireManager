#define IMGUI_DEFINE_MATH_OPERATORS

#include "globals.h"
D3DXVECTOR3 camOffset = { 1000.f, 1000.f, 1000.0f };
bool Dcheckbox = false;
bool enableAimbot = false;
bool perWeaponConfig = false;
bool kmbox_connected = false;
bool memwrite = false;
bool camera_hacks = false;
bool fast_knives = false;
bool super_kill = false;
float AimFov = 30;
float  AimSpeed = 0.04f;
float MaxAimDistance = 50;
float radarRotationDeg = 90.0f;
float radarRadius = 100.0f;
float radarMaxDistance = 100.0f;
bool  draw_enemy_names = false;
float  radar_size_factor = 0.2f;
float radar_bg_alpha = 0.1f;
float radar_opacity = 0.1f;
float radar_elev_threshold = 1.0f;
float radar_zoom = 1.3f;
float color[4] = { 221 / 255.f, 255 / 255.f, 11 / 255.f, 1.f };
int showhidekey;
int firstHotkey = 0;
int smoothing = 5;
int aimkey = 0;
int keymode = 0;
int selectedSmoothType = 0;
int selectedAimDevice = 0;
int MemoryMode = 0;
int ToggleorHold = 0;
int AimPosition = 0;
int TargetSwitch = 0;
int selectedPerspective = 0;
bool crosshair_notify = false;
bool Headcheckbox = false;
bool Healthcheckbox = false;
bool showInfoText = true;
bool Namecheckbox = false;
bool Distancecheckbox = false;
bool weaponcheckbox = false;
bool showFOVCircle = false;
bool Filterteams = false;
bool showFPS = true;
bool draw_radar = false;
bool Bonecheckbox = false;
bool draw_enemies_as_arrows = false;
bool showEspLines = false;
int esptype = 0;
float boxtk = 2.5f;
float hptk = 2.1f;
float hdtk = 5.f;
float bonetk = 1.f;
bool Flogs[5] = { false, false, false, false, false };
const char* Flogss[5] = { "Head", "Health Bar", "Player Name", "Distance", "Extra" };
 

int fov = 300;
RGBA g_EnemyColor{ 255, 0, 0, 255 };
RGBA g_AllyColor{ 0, 255, 0, 255 };
RGBA g_ESPLineColor{ 255, 255, 255, 255 };
RGBA g_NameColor{ 255, 255, 0, 255 };
RGBA g_HeadColor{ 255, 255, 255, 255 };
 