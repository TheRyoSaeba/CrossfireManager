#pragma once
#define NOMINMAX
#include <nlohmann/json.hpp>
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <string_view>
#include "Memory.h"
#include "CacheManager.h"
#include "imgui.h"
#include "../Config/config.h"
#include "../Config/Globals.h"
#include "../Classes.h"
#include <d3dx9math.h> 
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include <mutex>
#include <vector>
#include <tchar.h>
#include <d3d9.h>
#include "cache.h"
#include <d3dx9.h>


 
struct RectData {
    float x, y, w, h;
    RGBA color;
    std::string playerName;
    int currentHP, maxHP;
    D3DXVECTOR3 headPos;
    bool isAlly;
};

using namespace KLASSES;

 
namespace ESP {
    void Render(Memory& mem, std::shared_ptr<ESP::Snapshot> snapshot, ImDrawList* draw);
    void DrawHeadCircle(const RectData& rect, ImDrawList* draw, const LT_DRAWPRIM& drawPrim);
    void DrawCornerBox(int x, int y, int w, int h, float borderPx, RGBA color);
    void DrawBoxESP(const RectData& rect, ImDrawList* draw, float scaleFactor, int esptype);
    void DrawTraceline(const RectData& rect, const ESP::Snapshot& snapshot, ImDrawList* draw);
    void DrawHealthBar(const RectData& rect, float health, ImDrawList* draw);
    void DrawNameESP(const RectData& rect, ImDrawList* draw);
    void DrawDistance(const RectData& rect, float distance, ImDrawList* draw);
    void DrawBones(Memory& mem, const ESP::Snapshot& snapshot, ImDrawList* draw);

}
 
