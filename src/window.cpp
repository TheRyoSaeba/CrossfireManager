#pragma once
#include <windows.h>
#include "window.h"
#include "ESPManager.hpp"
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <shlobj_core.h>
#include <direct.h>
#include <fstream>
#include <mutex>
#include <vector>
#include <Overlay.h>
#include <offsets.h>

#ifndef RGB
#define RGB(r, g, b) ((DWORD)(((BYTE)(r) | ((WORD)(g) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#endif

using namespace offs;
// allow timeBeginPeriod and timeEndPeriod
#pragma comment(lib, "Winmm.lib")
static std::vector<RectData> cachedRects;
bool window::is_minimized() {
    return !window::hwnd || IsIconic(window::hwnd);
}
static bool lastESPState = false;

void window::render_frame() {
    if (fonts_need_rebuild) {
        fonts_need_rebuild = false;
        clean_up();
        setup_graphics(hwnd);
    }

    if (default_font != NULL && default_font->ptr != NULL) {
        ImGui::GetIO().FontDefault = default_font->ptr;
    }

    // ✅ Start the ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

     
    if (ESPDRAWBOX) {
        if(!ESPManager::GetInstance().IsESPActive())
        ESPManager::GetInstance().StartESP(mem);

        ImGui::Begin("MakimuraDMA", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);

        window::outlined_text(ImVec2(50, 50), IM_COL32(255, 0, 0, 255), "ESP Overlay ON");
        ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);

        ImGui::End();

        std::vector<RectData> rectsCopy;
        {
            std::lock_guard<std::mutex> lock(g_espMutex);
            rectsCopy = g_espRects;
        }

        auto drawList = ImGui::GetForegroundDrawList();

        for (const auto& rect : rectsCopy) {

            int adjustedX = rect.x - 5;
            int adjustedY = rect.y - 1;
            int adjustedW = rect.w + 1;
            int adjustedH = rect.h + 1;

            drawList->AddRect(

                ImVec2(static_cast<float>(adjustedX), static_cast<float>(adjustedY)),
                ImVec2(static_cast<float>(adjustedX + adjustedW), static_cast<float>(adjustedY + adjustedH)),
                IM_COL32(rect.color.R, rect.color.G, rect.color.B, rect.color.A),
                0.0f, ImDrawFlags_Closed, 1.8f
            );

            ImVec2 textSize = ImGui::CalcTextSize(rect.playerName.c_str());
            float nameX = rect.x + (rect.w - textSize.x) * 0.5f;
            float nameY = rect.y - textSize.y - 2.0f;
            drawList->AddText(ImVec2(nameX, nameY), IM_COL32(255, 255, 255, 255), rect.playerName.c_str());int hp = std::max(0, rect.currentHP);
            float healthPercent = static_cast<float>(hp) / static_cast<float>(rect.maxHP);
            float barWidth = 4.0f;
            float barHeight = rect.h * healthPercent;
            float barX = rect.x - (barWidth + 3.0f);
            float barY = rect.y + (rect.h - barHeight);
            drawList->AddRectFilled(
                ImVec2(barX, barY),
                ImVec2(barX + barWidth, barY + barHeight),
                GetHealthColor(healthPercent)
            );


        }
    }
    else {
        ESPManager::GetInstance().StopESP();
        {
            window::outlined_text(ImVec2(50, 50), IM_COL32(255, 0, 0, 255), "ESP Overlay OFF");
            std::lock_guard<std::mutex> lock(g_espMutex);
            g_espRects.clear();
            cachedRects.clear();
        }
    }

    


    ImGui::EndFrame();
    ImGui::Render();
 




    const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    // Present the frame
    g_pSwapChain->Present(vsync == 1 ? 1 : 0, 0);

    if (!vsync && fps_limit > 0) {
        static auto lastFrameTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        double elapsedTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();

        const double frameTime = 1.0 / (double)fps_limit;
        if (elapsedTime < frameTime) {
            DWORD sleepTime = static_cast<DWORD>((frameTime - elapsedTime) * 1000);

            timeBeginPeriod(1);
            Sleep(sleepTime);
            timeEndPeriod(1);
        }

        lastFrameTime = std::chrono::high_resolution_clock::now();
    }
}

LRESULT CALLBACK window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;

        window::wx = (UINT)LOWORD(lParam);
        window::wy = (UINT)HIWORD(lParam);
        resize_width = (UINT)LOWORD(lParam);
        resize_height = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
        // on window minimize clean up imgui and directx and reinitialize
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void window::set_mouse_passthrough(bool state) {
    if (allow_clicks == state) return;

    allow_clicks = state;
    if (state) {
        SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLongA(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
    }
    else {
        SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLongA(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    }
}

void window::clean_render_target() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void window::clean_up() {
    //std::cout << xorstr_("[!] Cleaning window\n");

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    clean_render_target();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void window::create_render_target() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void window::setup_graphics(HWND hwnd) {
    // Create the Direct3D device and swap chain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 2;
    scd.BufferDesc.Width = 0;
    scd.BufferDesc.Height = 0;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 144;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;

    auto result = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
        D3D11_SDK_VERSION, &scd, &g_pSwapChain, &g_pd3dDevice, NULL,
        &g_pd3dDeviceContext);

    if (result != S_OK) {

        return;
    }

    create_render_target();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // add if dont exist alr
    load_font(xorstr_("arial"), 16, false, false);
    load_font(xorstr_("arial"), 14, false, false);

    ImGuiIO io = ImGui::GetIO();

    // add all fonts from font_map
    for (int i = 0; i < font_list.size(); i++) {
        auto font = font_list[i];

        if (!std::filesystem::exists(font->path)) {
            if (font == selected_font) {
                selected_font = nullptr;
            }

            if (font == default_font) {
                default_font = NULL;
                io.FontDefault = NULL;
            }

            // remove font from list
            font_list.erase(font_list.begin() + i);
            i--;

            delete font;
            continue;
        }

        font->ptr = io.Fonts->AddFontFromFileTTF(font->path.c_str(), font->size);
    }

    static bool first_load = false;
    if (first_load || io.FontDefault == NULL) {
        first_load = false;

        auto font = get_font_by_name(xorstr_("arial"), 16);
        io.FontDefault = font ? font->ptr : io.Fonts->AddFontDefault();
    }

    if (first_load) {
        auto sfont = get_font_by_name(xorstr_("arial"), 14);
        window::small_font = sfont ? sfont->ptr : io.FontDefault;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 1.0f;  // UI elements visible
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0.0f);  // No background
    style.FrameBorderSize = 0.f;
    style.FrameRounding = 4.f;
    style.GrabMinSize = 4.f;
    style.GrabRounding = 2.f;
    //style.WindowPadding = ImVec2(0, 0);

    current_hue = round((201.f / 255.f) * 100.f) / 100.f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.79f, 0.26f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.79f, 0.26f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.39f, 0.16f, 0.48f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.78f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.74f, 0.12f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.60f, 0.21f, 0.74f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.14f, 0.57f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.71f, 0.06f, 0.95f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.79f, 0.26f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.79f, 0.26f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.79f, 0.26f, 0.98f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.57f, 0.10f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.57f, 0.10f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.79f, 0.26f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.79f, 0.26f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.79f, 0.26f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.47f, 0.18f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.79f, 0.26f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.55f, 0.20f, 0.68f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.79f, 0.26f, 0.98f, 0.35f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.79f, 0.26f, 0.98f, 1.00f);

    if (window::styling_hook != NULL) {
        reinterpret_cast<void(*)()>(styling_hook)();
    }
}

void window::init(bool* running) {

    // ✅ Use consistent `std::wstring`
    const std::wstring title = L"BlurredDMA";

    window::wx = GetSystemMetrics(SM_CXSCREEN);
    window::wy = GetSystemMetrics(SM_CYSCREEN);
     
    // ✅ Use `WNDCLASSEXW` to ensure compatibility with Unicode
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WindowProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL, title.c_str(), NULL };

    if (!RegisterClassExW(&wc)) {
        std::wcout << L"[!] Failed to register window class: " << GetLastError() << L"\n";
        return;
    }

    // ✅ Use `CreateWindowExW()` with Unicode (`LPCWSTR`)
    hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED   | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE|WS_EX_LAYERED | WS_EX_TOOLWINDOW,  // Ensure transparency
        title.c_str(), title.c_str(),
        WS_POPUP,  // Removes title bar/borders
        0, 0, window::wx, window::wy,
        NULL, NULL, wc.hInstance, NULL
    );

    if (hwnd == NULL) {
        std::wcout << L"[!] Failed to create window: " << GetLastError() << L"\n";
        return;
    }



    // Ensure color key transparency (black background removed)
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);


    // Ensure window is borderless and covers the whole screen
  
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_APPWINDOW);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    //SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);


    if (icon_index > 0) {
        auto icon = LoadIcon(hInstance == NULL ? GetModuleHandle(NULL) : hInstance, MAKEINTRESOURCE(icon_index));
        if (icon) {
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
        }
    }

    if (target_monitor >= 0) {
        set_monitor(target_monitor);
    }

    // Initialize Direct3D and ImGui
    setup_graphics(hwnd);

    window::overlay = window::OverlayMode::TRANSPARENT_OVERLAY;
    window::update_overlay();

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT && *running) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (resize_width != 0 && resize_height != 0) {
            clean_render_target();
            g_pSwapChain->ResizeBuffers(0, resize_width, resize_height, DXGI_FORMAT_UNKNOWN, 0);
            resize_width = resize_height = 0;
            create_render_target();
        }

        // Render the frame
        render_frame();

        window::is_setup = true;
    }

    // Cleanup
    clean_up();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

void window::outlined_text(const ImVec2& pos, ImU32 color, const char* text) {
    /*if (pos.x < 0 || pos.y < 0 || pos.x > window::wx || pos.y > window::wy) {
        return;
    }*/

    ImDrawList* draw = ImGui::GetBackgroundDrawList();

    static const ImU32 back = ImColor(1, 1, 1);
    draw->AddText({ pos.x + 1, pos.y }, back, text);
    draw->AddText({ pos.x - 1, pos.y }, back, text);
    draw->AddText({ pos.x, pos.y + 1 }, back, text);
    draw->AddText({ pos.x, pos.y - 1 }, back, text);
    draw->AddText(pos, color, text);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);

    if (GetMonitorInfo(hMonitor, &monitorInfo)) {
        if (window::monitor_enum_state != window::target_monitor) {
            window::monitor_enum_state++;
            return TRUE;
        }
        SetWindowLong(window::hwnd, GWL_EXSTYLE, GetWindowLong(window::hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);

        // Set the window position and size to cover the entire monitor
        SetWindowPos(window::hwnd, nullptr,
            monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.top,
            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
            SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
        SetWindowPos(window::hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return FALSE;
    }

    return TRUE; // Continue enumerating
}

void window::set_monitor(int index) {
    window::target_monitor = index;
    window::monitor_enum_state = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}

void window::set_overlay(bool state, bool transparent) {
    if (state) {
        // Set window style for transparency
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOPMOST | WS_EX_LAYERED);

        if (transparent) {
            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        }
        else {
            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, NULL);
        }

        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return;
    }

    // clear all windows styles above
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_TOPMOST);
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, NULL);
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

ImFont* window::get_selected_font() {
    if (selected_font == NULL || selected_font->ptr == NULL) {
        return nullptr;
    }

    return selected_font->ptr;
}

font_data* window::load_font(std::string font, int sizeint, bool set_default, bool set_selected) {
    if (sizeint > 100 || sizeint < 1) {
        return nullptr;
    }

    float size = (float)sizeint;

    // convert font to lowercase
    std::transform(font.begin(), font.end(), font.begin(), ::tolower);

    bool was_path_input = font.ends_with(xorstr_(".ttf")) || font.ends_with(xorstr_(".otf"));

    std::string path = was_path_input ? font : xorstr_("C:\\Windows\\Fonts\\") + font + xorstr_(".ttf");
    if (!was_path_input && !std::filesystem::exists(path)) {
        path = xorstr_("C:\\Windows\\Fonts\\") + font + xorstr_(".otf");
    }

    if (!std::filesystem::exists(path)) {
        wchar_t localAppDataPath[MAX_PATH];
        SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataPath);

        std::wstring wstr(localAppDataPath);
        std::string str(wstr.begin(), wstr.end());

        path = str + xorstr_("\\Microsoft\\Windows\\Fonts\\") + font + xorstr_(".ttf");
        if (!std::filesystem::exists(path)) {
            path = str + xorstr_("\\Microsoft\\Windows\\Fonts\\") + font + xorstr_(".otf");
        }
    }

    for (auto& f : font_list) {
        if ((f->path == path || f->name == font) && f->size == size) {
            if (set_default) {
                default_font = f;
            }

            if (set_selected) {
                selected_font = f;
            }

            return f;
        }
    }

    if (!std::filesystem::exists(path)) {
        std::cout << xorstr_("[!] Font [") << font << xorstr_("] does not exist") << std::endl;
        return nullptr;
    }



    font_data* data = new font_data();
    data->name = font;
    data->path = path;
    data->size = size;

    font_list.push_back(data);

    if (set_default) {
        default_font = data;
    }

    if (set_selected) {
        selected_font = data;
    }

    fonts_need_rebuild = true;
    return data;
}

font_data* window::get_font_data(ImFont* imfont) {
    for (auto& font : font_list) {
        if (font->ptr == imfont) {
            return font;
        }
    }

    return NULL;
}

font_data* window::get_font_by_name(std::string name, float size) {
    // change name to lower
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    for (auto& font : font_list) {
        if ((font->name == name || font->path == name) && font->size == size) {
            return font;
        }
    }

    return NULL;
}

void window::push_disabled() {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void window::pop_disabled() {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

void window::tooltip(const char* tip) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(tip);
        ImGui::EndTooltip();
    }
}

void window::update_overlay() {
    set_overlay(overlay == OverlayMode::OVERLAY || overlay == OverlayMode::TRANSPARENT_OVERLAY, overlay == OverlayMode::TRANSPARENT_OVERLAY);
}

void window::kmbox_key_selector(const char* label, int* target, const std::vector<std::pair<std::string, int>>& keys) {
    std::string current_name = xorstr_("None");
    if (*target != 0) {
        for (auto const& [key, val] : keys) {
            if (*target == val) {
                current_name = key;
                break;
            }
        }
    }

    // set combo width to 100px
    ImGui::PushItemWidth(100);

    if (ImGui::BeginCombo(label, current_name.c_str())) {
        // sort the keys from lowest number to highest number then render them
        std::vector<std::pair<std::string, int>> myvec = keys;
        std::sort(myvec.begin(), myvec.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.second < rhs.second;
            });

        for (auto const& [key, val] : myvec) {
            bool selected = *target == val;
            if (ImGui::Selectable(key.c_str(), selected)) {
                *target = val;
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();
}
