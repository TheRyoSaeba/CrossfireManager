#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "imgui_freetype.h"
//#include <imgui_knobs.h>
#include <d3d11.h>
#include <tchar.h>

#include "notify.h"

#include "D3DX11tex.h"
#pragma comment(lib, "D3DX11.lib")

#include "image.h"
#include "font.h"
#include <dwmapi.h>
#include "KMBOX.h"
#include "Logging.h"
#include "../Memory/Memory.h"
#include "../region_header.h"
#include "../Config/globals.h"
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void set_mouse_passthrough(HWND hwnd);
void SetOverlayProperties(HWND hwnd, bool showMenu, int overlayMode);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
inline int tabs = 0;
inline  int overlayMode = 0;
extern bool showMenu;
inline int text_add = 0;
inline DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;
inline int target_monitor = -1;
inline bool checkbox = true;
inline bool checkbox_false = false;
inline int slider_int = 50;
inline float slider_float = 0.5f;
static int MOUSE_DPI = 1200;
static int GAME_SENS = 1;
inline int monitor_enum_state = 0;
inline int select_combo = 0;
inline const char* items[3]{ "dah one", "dah other one", "dah other other one" };

inline bool multi_num[5] = { false, false, false, false, false };
inline const char* multi_items[5] = { "One", "Two", "Three", "Four", "Five" };

inline std::vector<std::string> words;


static int  b;

static std::thread KeyCheckThread;
static std::atomic<bool> KeyCheckThreadRunning = false;
namespace font
{
    inline ImFont* calibri_bold = nullptr;
    inline ImFont* calibri_bold_hint = nullptr;
    inline ImFont* calibri_regular = nullptr;
	inline  ImFont* tahoma_bold = nullptr;
	inline ImFont* icomoon[50];
}

namespace texture
{
	inline ID3D11ShaderResourceView* background = nullptr;
	inline ID3D11ShaderResourceView* roughness = nullptr;
    inline  ID3D11ShaderResourceView* kaori_logo = nullptr;
	inline ID3D11ShaderResourceView* arrow = nullptr;
}
 


inline BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
    LPRECT lprcMonitor, LPARAM dwData) {
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);

    if (GetMonitorInfo(hMonitor, &monitorInfo)) {
        if (monitor_enum_state != target_monitor) {
            monitor_enum_state++;
            return TRUE;
        }

        HWND hwnd = reinterpret_cast<HWND>(dwData);

        
        SetWindowPos(hwnd, NULL, monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.top,
            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

        
        SetOverlayProperties(hwnd, showMenu, overlayMode);

        return FALSE;
    }
    return TRUE;
}

inline void SetOverlayProperties(HWND hwnd, bool showMenu, int overlayMode)
{
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);

    // Always keep TOOLWINDOW to hide from taskbar
    style |= WS_EX_TOOLWINDOW;

    if (overlayMode == 0) {
        // Transparent Mode
     

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        if (showMenu) {
            style &= ~WS_EX_LAYERED;
            style &= ~WS_EX_TRANSPARENT;  // Capture mouse for menu interaction
            style &= ~WS_EX_NOACTIVATE;   // Allow keyboard focus

            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE);  
        }
        else {
            style |= WS_EX_LAYERED;
            style |= WS_EX_TRANSPARENT;   // Allow clicks to pass through to game
            style |= WS_EX_NOACTIVATE;    // Prevent focus stealing

            // Keep TOPMOST so ESP remains visible over game
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    else {
        // Fuser Mode
        style &= ~WS_EX_LAYERED;  // No transparency for black background

        MARGINS margins = { 0, 0, 0, 0 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        if (showMenu) {
            // When menu is visible:
            style &= ~WS_EX_TRANSPARENT;  // Capture mouse
            style &= ~WS_EX_NOACTIVATE;   // Allow keyboard focus

            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE);
        }
        else {
            // When menu is hidden:
            style |= WS_EX_TRANSPARENT;   // Pass mouse clicks
            style |= WS_EX_NOACTIVATE;    // Prevent focus stealing

            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }

    SetWindowLong(hwnd, GWL_EXSTYLE, style);
}

 
inline void set_mouse_passthrough(HWND hwnd) {
    SetOverlayProperties(hwnd, showMenu, overlayMode);
}

inline void EnableGlassTransparency(HWND hwnd, bool enable) {
    overlayMode = enable ? 0 : 1;
    SetOverlayProperties(hwnd, showMenu, overlayMode);
}
 

inline void RGBAtoFloat4(const RGBA& rgba, float* colorFloat4) {
    colorFloat4[0] = rgba.R / 255.0f;
    colorFloat4[1] = rgba.G / 255.0f;
    colorFloat4[2] = rgba.B / 255.0f;
    colorFloat4[3] = rgba.A / 255.0f;
}
inline RGBA Float4toRGBA(const float* colorFloat4) {
    RGBA rgba;
    rgba.R = static_cast<int>(colorFloat4[0] * 255.0f);
    rgba.G = static_cast<int>(colorFloat4[1] * 255.0f);
    rgba.B = static_cast<int>(colorFloat4[2] * 255.0f);
    rgba.A = static_cast<int>(colorFloat4[3] * 255.0f);
    return rgba;
}
inline bool get_kmbox_version(HANDLE hSerial, std::string& out_device_type, int timeout_ms = 2000) {
    send_command(hSerial, "km.version()\r");

    auto start_time = std::chrono::steady_clock::now();
    std::string response;
    char buffer[256];
    DWORD bytesRead;

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;


            if (response.find(">>>") != std::string::npos)
                break;
        }

        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeout_ms) {
            return false;
        }
    }


    if (response.find("km.MAKCU") != std::string::npos) {
        out_device_type = "MAKCU";
        return true;
    }
    else if (response.find("km.") != std::string::npos) {
        out_device_type = "KMBOX";
        return true;
    }

    return false;
}

enum KMBOX_INIT_RESULT {
    KMBOX_INIT_SUCCESS = 0,
    KMBOX_DEVICE_NOT_FOUND = -1,
    KMBOX_PORT_NOT_OPEN = -2,
    KMBOX_UNKNOWN_ERROR = -3
};

inline void attempt_kmbox_connection() {
    int kmResult = kmBoxBMgr.init();

    switch (kmResult) {
    case KMBOX_DEVICE_NOT_FOUND:
        ImGui::InsertNotification({ ImGuiToastType_Error, 2500, "KMBOX BPRO/MAKCU device not found." });
        kmbox_connected = false;
        return;

    case KMBOX_PORT_NOT_OPEN:
        ImGui::InsertNotification({ ImGuiToastType_Error, 2500, "KMBOX Port not open." });
        kmbox_connected = false;
        return;

    case KMBOX_INIT_SUCCESS:
        ImGui::InsertNotification({ ImGuiToastType_Info, 1500, "KMBOX initialized, verifying device..." });
        break;

    default:
        ImGui::InsertNotification({ ImGuiToastType_Error, 2500, "Unknown KMBOX error." });
        kmbox_connected = false;
        return;
    }

    HANDLE hSerial = kmBoxBMgr.getSerialHandle();
    if (hSerial == INVALID_HANDLE_VALUE) {
        ImGui::InsertNotification({ ImGuiToastType_Error, 2500, "Invalid serial handle." });
        kmbox_connected = false;
        return;
    }

    std::string device_type;
    if (!get_kmbox_version(hSerial, device_type)) {
        ImGui::InsertNotification({ ImGuiToastType_Error, 2500, "Failed to identify device." });
       
        kmbox_connected = false;
        return;
    }

    if (device_type == "MAKCU") {
        ImGui::InsertNotification({ ImGuiToastType_Success, 2500, "MAKCU Device Connected." });
        kmbox_connected = true;
    }
    else if (device_type == "KMBOX") {
        ImGui::InsertNotification({ ImGuiToastType_Success, 2500, "KMBOX Device Connected." });
        kmbox_connected = true;
    }
    else {
        ImGui::InsertNotification({ ImGuiToastType_Error, 2500, "Unrecognized KMBOX device." });
        
        kmbox_connected = false;
    }
}

 

inline void  set_monitor(int index, HWND hwnd) {
    target_monitor = index;
    monitor_enum_state = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc,
        reinterpret_cast<LPARAM>(hwnd));
}



   struct OverlayInitData {
    HWND hwnd = nullptr;
    WNDCLASSEXW wc{};
};

   inline void StartKeyCheck(HWND hwnd)
   {

      bool isDown = (GetAsyncKeyState(showhidekey) & 0x8000) || mem.GetKeyboard()->IsKeyDown(showhidekey);

           if (isDown)
            {
                
                showMenu = !showMenu;
              set_mouse_passthrough(hwnd);
                      
            }
            
              
               }

inline OverlayInitData SetupImGuiAndWindow()
{
    OverlayInitData data;

    data.wc = {
        sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"Makimura", nullptr
    };

    if (!::RegisterClassExW(&data.wc))
        return {};

    data.hwnd = CreateWindowExW(
        WS_EX_TOPMOST |WS_EX_LAYERED | WS_EX_TOOLWINDOW ,
        data.wc.lpszClassName,
        L"Makimura",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, data.wc.hInstance, nullptr);

    if (!data.hwnd)
        return {};

    
     
    EnableGlassTransparency(data.hwnd, true);

     set_monitor(0, data.hwnd);

    if (!CreateDeviceD3D(data.hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(data.wc.lpszClassName, data.wc.hInstance);
        return {};
    }

    ::ShowWindow(data.hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(data.hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(data.hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return data;
}

inline  void  InitMenuFontsAndTextures()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImFontConfig cfg;
    ImFontConfig cfg_regular;

    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;
    cfg_regular.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

    font::calibri_bold = io.Fonts->AddFontFromMemoryTTF(calibri_bold, sizeof(calibri_bold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::calibri_bold_hint = io.Fonts->AddFontFromMemoryTTF(calibri_bold, sizeof(calibri_bold), 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::calibri_regular = io.Fonts->AddFontFromMemoryTTF(calibri_regular, sizeof(calibri_regular), 12.f, &cfg_regular, io.Fonts->GetGlyphRangesCyrillic());

    for (int i = 14; i < 45; i++)
        font::icomoon[i] = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), i, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    font::tahoma_bold = io.Fonts->AddFontFromMemoryTTF(tahoma_bold, sizeof(tahoma_bold), 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());


    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    ImGui_ImplDX11_CreateDeviceObjects();


    D3DX11_IMAGE_LOAD_INFO info;
    ID3DX11ThreadPump* pump = nullptr;
    if (texture::background == nullptr)
        D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, background_image, sizeof(background_image), &info, pump, &texture::background, 0);
    if (texture::roughness == nullptr)
        D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, kaori_logo, sizeof(logo), &info, pump, &texture::roughness, 0);
    if (texture::kaori_logo == nullptr)
        D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, kaori_logo, sizeof(kaori_logo), &info, pump, &texture::kaori_logo, 0);
}
 


inline void DisplayFPS()
{
    if (showInfoText)
    {
        char buffer[64];
        sprintf_s(buffer, sizeof(buffer), "FPS: %d", (int)ImGui::GetIO().Framerate);
        ImGui::InsertNotification({ ImGuiToastType_Success, 15000, buffer });
    }

    
}
namespace MainThread {
	 int Render_Loop();
}

 