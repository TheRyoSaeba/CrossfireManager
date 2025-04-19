#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX

#include <nlohmann/json.hpp>
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <string_view>
#include "Memory.h"
#include "imgui.h"
#include "../Config/Globals.h"
#include "../region_header.h"
#include <d3dx9math.h> 
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include <mutex>
#include <vector>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "../Cache/Cache.h"
#include "../Cache/CacheManager.h"
#include <DirectXMath.h>

using namespace KLASSES;

struct RectData {
    float x, y, w, h;
    RGBA color;
    std::string playerName;
    int currentHP, maxHP;
    D3DXVECTOR3 headPos;
    bool isAlly;
};
 
struct AimContext {
    std::shared_ptr<ESP::Snapshot> snapshot;
    ImDrawList* drawList;
    D3DXVECTOR3 targetPos = { 0.f, 0.f, 0.f };
};

 

 
namespace ESP {
    void Render(Memory& mem, std::shared_ptr<ESP::Snapshot> snapshot, ImDrawList* draw);
    void DrawHeadCircle(const RectData& rect, ImDrawList* draw, const LT_DRAWPRIM& drawPrim);
    void DrawCornerBox(int x, int y, int w, int h, float borderPx, RGBA color);
    void DrawBoxESP(const RectData& rect, ImDrawList* draw, float scaleFactor, int esptype);
    void DrawTraceline(const RectData& rect, const ESP::Snapshot& snapshot, ImDrawList* draw);
    void DrawHealthBar(const RectData& rect, float health, ImDrawList* draw);
    void DrawNameESP(const RectData& rect, ImDrawList* draw);
    void Draw3DBox(ImDrawList* draw, const D3DXVECTOR3& footPos, const D3DXVECTOR3& headPos,
        const KLASSES::LT_DRAWPRIM& drawPrim, ImU32 color);
    float RadarAngle(float delta);
    void DrawBonePreview(ImDrawList* draw, const ImVec2& origin, float scale, ImU32 color);
    void DrawDistance(const RectData& rect, float distance, ImDrawList* draw);
    void DrawBones(Memory& mem, const ESP::Snapshot& snapshot, ImDrawList* draw);
    void RenderRadar(Memory& mem, std::shared_ptr<ESP::Snapshot> snapshot, ImDrawList* draw);
    void DrawFOVCircle(ImDrawList* draw, const KLASSES::LT_DRAWPRIM& drawPrim, float AimFov, ImU32 color);
    void DebugDrawFOVDetection(const std::shared_ptr<ESP::Snapshot>& snapshot, ImDrawList* draw);
    void DrawWorldLogo(ImDrawList* draw, const KLASSES::LT_DRAWPRIM& drawPrim, std::shared_ptr<ESP::Snapshot> snapshot);
    void ESP::DebugDrawFOVDetection(const std::shared_ptr<ESP::Snapshot>& snapshot, ImDrawList* draw);
    D3DXVECTOR3 NormalizeVec3(const D3DXVECTOR3& vec);
}

 
