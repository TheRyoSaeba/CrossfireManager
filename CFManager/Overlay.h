#pragma once
#include <nlohmann/json.hpp>
 
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include <mutex>
#include <vector>
#include <tchar.h>
#include <d3d9.h>

#ifdef RGBA
#undef RGBA
#endif

#include "Classes.h"
#include "cache.h"

using json = nlohmann::json;

struct RGBA {
    int R;
    int G;
    int B;
    int A;
};


struct RectData {
    float x;
    float y;
    float w;
    float h;
    RGBA color;
    std::string playerName;
    int currentHP;
    int maxHP;
    D3DXVECTOR3 headPos;
    bool isAlly;
};

 
struct LineData {
    int x1;
    int y1;
    int x2;
    int y2;
    ImU32 color;
};

inline std::mutex g_espMutex;
inline std::vector<RectData> g_espRects;

inline std::string string_To_UTF8(const std::string& str) {
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
    wchar_t* pwBuf = new wchar_t[nwLen];
    ZeroMemory(pwBuf, nwLen * sizeof(wchar_t));
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, nullptr, 0, nullptr, nullptr);
    char* pBuf = new char[nLen];
    ZeroMemory(pBuf, nLen);
    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, nullptr, nullptr);
    std::string retStr(pBuf);
    delete[] pwBuf;
    delete[] pBuf;
    return retStr;
}
inline std::string formatBaudRate(DWORD baudRate) {
    if (baudRate >= 1000000) {
        return std::to_string(baudRate / 1000000) + "Mbps";   
    }
    return std::to_string(baudRate / 1000) + "k";   
}

inline ImU32 GetHealthColor(float healthPercent) {
    int r, g, b;
    if (healthPercent > 0.5f) {

        float t = (healthPercent - 0.5f) / 0.5f;
        r = static_cast<int>(255 * (1 - t));
        g = 255;
    }
    else {

        float t = healthPercent / 0.5f;
        r = 255;
        g = static_cast<int>(255 * t);
    }
    b = 0;
    return IM_COL32(r, g, b, 255);
}



inline float mid(float a, float b) {
    return (a + b) * 0.5f;
}


void DrawAllBones(
    ImDrawList* draw,
    Memory& mem,
    uintptr_t hObject,
    const std::vector<int>& boneIDs,
    const std::array<D3DXVECTOR3, NUM_BONES>& positions,
    const KLASSES::LT_DRAWPRIM& drawPrim,
    ImU32 color)
{
    static std::vector<std::pair<int, int>> bonePairs = {
        {1, 3}, {3, 4}, {4, 5}, {5, 6},
        {4, 7}, {7, 8}, {8, 9}, {9, 10},
        {4, 14}, {14, 15}, {15, 16}, {16, 17},
        {1, 21}, {21, 22}, {22, 23},
        {1, 25}, {25, 26}, {26, 27}
    };

    for (auto& bp : bonePairs)
    {
        int boneA = bp.first;
        int boneB = bp.second;

        
        auto itA = std::find(boneIDs.begin(), boneIDs.end(), boneA);
        if (itA == boneIDs.end()) continue;
        size_t idxA = std::distance(boneIDs.begin(), itA);

        auto itB = std::find(boneIDs.begin(), boneIDs.end(), boneB);
        if (itB == boneIDs.end()) continue;
        size_t idxB = std::distance(boneIDs.begin(), itB);

        
        D3DXVECTOR3 posA = positions[idxA];
        D3DXVECTOR3 posB = positions[idxB];

        posA.y += 5.0f;
        posB.y += 5.0f;

        if (!KLASSES::EngineW2S(drawPrim, &posA))
            continue;
        if (!KLASSES::EngineW2S(drawPrim, &posB))
            continue;
        float bonethick = 1.5 + bonetk;
        draw->AddLine(
            ImVec2(posA.x, posA.y),
            ImVec2(posB.x, posB.y),
            color,
            bonethick
        );
    }
}

inline void DrawCornerBox(int x, int y, int w, int h, float borderPx, RGBA color) {
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    ImU32 col = IM_COL32(color.R, color.G, color.B, color.A);

    float lineW = (w / 8.0f);
    float lineH = (h / 8.0f);

    draw->AddLine(ImVec2(x, y), ImVec2(x + lineW, y), col, borderPx);
    draw->AddLine(ImVec2(x, y), ImVec2(x, y + lineH), col, borderPx);

    draw->AddLine(ImVec2(x + w, y), ImVec2(x + w - lineW, y), col, borderPx);
    draw->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + lineH), col, borderPx);

    draw->AddLine(ImVec2(x, y + h), ImVec2(x, y + h - lineH), col, borderPx);
    draw->AddLine(ImVec2(x, y + h), ImVec2(x + lineW, y + h), col, borderPx);

    draw->AddLine(ImVec2(x + w, y + h), ImVec2(x + w - lineW, y + h), col, borderPx);
    draw->AddLine(ImVec2(x + w, y + h), ImVec2(x + w, y + h - lineH), col, borderPx);
}

std::vector<D3DXVECTOR3> GetAllBones(Memory& mem, uintptr_t hObject,
    const std::vector<int>& boneIDs) {

    std::vector<D3DXVECTOR3> results(boneIDs.size());

    auto scatterHandle = mem.CreateScatterHandle();
    if (!scatterHandle) {

        return results;
    }

    uintptr_t boneArrayAddr = hObject + offsetof(KLASSES::obj, BoneArray);
    uintptr_t boneArray = mem.Read<uintptr_t>(boneArrayAddr);
    if (!boneArray) {
        mem.CloseScatterHandle(scatterHandle);
        return results;
    }

    struct BoneScatter {
        D3DXMATRIX mat;
    };
    std::vector<BoneScatter> localMats(boneIDs.size());

    for (size_t i = 0; i < boneIDs.size(); i++) {
        int bone = boneIDs[i];
        uintptr_t matrixAddr = boneArray + (bone * sizeof(D3DXMATRIX));

        mem.AddScatterReadRequest(scatterHandle, matrixAddr, &localMats[i].mat,
            sizeof(D3DXMATRIX));
    }

    mem.ExecuteReadScatter(scatterHandle);

    mem.CloseScatterHandle(scatterHandle);

    for (size_t i = 0; i < boneIDs.size(); i++) {
        results[i].x = localMats[i].mat._14;
        results[i].y = localMats[i].mat._24;
        results[i].z = localMats[i].mat._34;
    }

    return results;
}

inline void DrawRect(int x, int y, int w, int h, const RGBA* color, int thickness) {
    auto drawList = ImGui::GetForegroundDrawList();
    if (!drawList) return;
    drawList->AddRect(
        ImVec2(static_cast<float>(x), static_cast<float>(y)),
        ImVec2(static_cast<float>(x + w), static_cast<float>(y + h)),
        IM_COL32(color->R, color->G, color->B, color->A),
        0.0f,
        ImDrawFlags_Closed,
        static_cast<float>(thickness)
    );
}