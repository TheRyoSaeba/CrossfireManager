#include "globals.h"

bool Dcheckbox = false;
bool enableAimbot = false;
float AimFov = 100;
float AimSpeed = 0.5f;
float MaxAimDistance = 100;
int firstHotkey = 0;
int selectedSmoothType = 0;
bool crosshair_notify = false;
bool Headcheckbox = false;
bool Healthcheckbox = false;
bool showInfoText = true;
bool Namecheckbox = false;
bool Distancecheckbox = false;
bool weaponcheckbox = false;
bool Filterteams = false;
bool showFPS = true;
bool draw_radar = false;
bool Bonecheckbox = false;
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