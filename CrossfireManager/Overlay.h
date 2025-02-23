#pragma once

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include <mutex>
#include <vector>
#include <tchar.h>
#include <d3d9.h>
#include "Classes.h"

// First, define RGBA
struct RGBA {
    int R;
    int G;
    int B;
    int A;
};

// Use inline constexpr so these variables have external linkage but are defined in the header.
// (Make sure your project is set to compile with C++17 or later.)
inline constexpr RGBA White{ 255, 255, 255, 255 };
inline constexpr RGBA Red{ 255, 0, 0, 255 };
inline constexpr RGBA Green{ 0, 255, 0, 255 };
inline constexpr RGBA Green2{ 0, 0, 0, 255 };
inline constexpr RGBA Yellow{ 255, 255, 0, 255 };
inline constexpr RGBA ESPColorEnemy{ 255, 0, 0, 255 }; // Red
inline constexpr RGBA ESPColorAlly{ 0, 255, 0, 255 };  // Green

// Define RectData structure that uses RGBA.
struct RectData {
    int x;
    int y;
    int w;
    int h;
    RGBA color;
    std::string playerName;
    int currentHP;
    int maxHP;
};

struct LineData {
    int x1;
    int y1;
    int x2;
    int y2;
    ImU32 color;
};

// Now, define the globals as inline variables so they have a single definition.
inline std::mutex g_espMutex;
inline std::vector<RectData> g_espRects;

// Utility: Convert string to UTF8.
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

inline ImU32 GetHealthColor(float healthPercent) {
    int r, g, b;
    if (healthPercent > 0.5f) {
        // Health between 50% and 100%: interpolate from yellow (255,255,0) to green (0,255,0)
        float t = (healthPercent - 0.5f) / 0.5f; // t goes from 0 to 1
        r = static_cast<int>(255 * (1 - t)); // r goes from 255 to 0
        g = 255;                          // g remains 255
    }
    else {
        // Health between 0% and 50%: interpolate from red (255,0,0) to yellow (255,255,0)
        float t = healthPercent / 0.5f;    // t goes from 0 to 1
        r = 255;                          // r remains 255
        g = static_cast<int>(255 * t);    // g goes from 0 to 255
    }
    b = 0; // always 0
    return IM_COL32(r, g, b, 255);
}

// Drawing function that uses ImGui's drawing API.
inline void DrawRect(int x, int y, int w, int h, const RGBA* color, int thickness) {
    auto drawList = ImGui::GetForegroundDrawList(); // Draw on the foreground layer.
    if (!drawList) return;
    drawList->AddRect(
        ImVec2(static_cast<float>(x), static_cast<float>(y)),
        ImVec2(static_cast<float>(x + w), static_cast<float>(y + h)),
        IM_COL32(color->R, color->G, color->B, color->A),
        0.0f, // No rounding.
        ImDrawFlags_Closed,
        static_cast<float>(thickness)
    );
}
