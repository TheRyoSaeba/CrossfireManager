#define IMGUI_DEFINE_MATH_OPERATORS
 
#define NOMINMAX  
#include "ESP/ESP.h"
#include "Config/globals.h"
#include "offsets.h"
#include "Config/config.h"
 #include <Windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "imgui_freetype.h"
#include <d3d11.h>
#include <tchar.h>
#include <algorithm>
#include <atomic>
#include <algorithm>
#include "auth.hpp" 
#include "../Include/skStr.h"
#include "../Include/VMProtectSDK.h"
#include <imgui.h>
#include <vector>
#include <array>
#include <mutex>
#include <iostream>
#include <string>
#include <regex>
#include <future>
#include <mutex>
#include "Memory/Memory.h"
#include "KMBOX.h"
#include <iostream>
#include "D3DX11tex.h"
 #pragma comment(lib, "D3DX11.lib")
#include "imgui_settings.h"
#include "font.h"
#include "image.h"
#include <dwmapi.h>
#include <pch.h>
#include "Cache.h"
#include "CacheManager.h"
#include <DirectXMath.h>  
 
 
using namespace DirectX;

using namespace KeyAuth;
  
using namespace std::chrono_literals;
 
CacheManager g_cacheManager;  
inline std::jthread bgThread;
 
 

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace font
{
    ImFont* poppins_medium = nullptr;
    ImFont* poppins_medium_low = nullptr;
    ImFont* tab_icon = nullptr;
    ImFont* chicons = nullptr;
    ImFont* tahoma_bold = nullptr;
    ImFont* tahoma_bold2 = nullptr;
}
namespace image
{
    ID3D11ShaderResourceView* bg = nullptr;
    ID3D11ShaderResourceView* logo = nullptr;
    ID3D11ShaderResourceView* logo_general = nullptr;

    ID3D11ShaderResourceView* arrow = nullptr;
    ID3D11ShaderResourceView* bell_notify = nullptr;
    ID3D11ShaderResourceView* roll = nullptr;


}


struct ScopedStyleColor {
    ScopedStyleColor(ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
    ~ScopedStyleColor() { ImGui::PopStyleColor(1); }
};
D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;
float tab_size = 0.f;
float arrow_roll = 0.f;
bool tab_opening = true;
int rotation_start_index;
 
static int selectedSmoothType = 0;
 
static bool isMenuFocused = false;
inline int target_monitor = -1;
inline int fps_limit = 144;
inline int icon_index = 0;
 
inline HINSTANCE hInstance = NULL;
static std::future<bool> updateFuture;
inline int monitor_enum_state = 0;
 
inline bool allow_clicks = false;
 



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

static std::string kmbox_status = "Initializing...";
static bool kmbox_connected = false;
static DWORD connectedBaudRate = 0;
static bool attempted_connection = false;
static bool autoReconnect = false;
static bool showMenu = true;
static bool overlayVisible = true;
static int overlayMode = 0;
bool RUNCACHE = false;
static bool g_hasFocus = false;
static std::mutex status_mutex;
static std::future<void> update_task;
static std::atomic<bool> update_in_progress{ false };
static std::string dma_status = "Not Initialized";
static std::string g_sanitizedKey;
static std::atomic<bool> dma_success(false);
static int initialization_attempts = 0;
static bool initializing_dma = false;
 
 
void ImRotateStart()
{
    rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
}
 
 
std::string sanitizeLicense(const std::string& license) {
    std::regex validPattern("^CROSSFIRE-[A-Za-z0-9]{6}(?:-[A-Za-z0-9]{6}){3}$", std::regex_constants::icase);
    if (std::regex_match(license, validPattern))
        return license;
    return "";
}





void  full_refresh() {
     
        VMMDLL_ConfigSet(mem.vHandle, VMMDLL_OPT_REFRESH_ALL, 1);
       
}
void InitializeDMA(Memory& mem) {
    initializing_dma = true;
    initialization_attempts = 0;
    dma_status = "Connecting...";

    std::thread([&]() {
        for (int attempts = 0; attempts < 3; ++attempts) {
            if (!mem.Init("crossfire.exe",true)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else {
                LOG("[!]Finished Connecting to DMA, Grabbing Crossfire.exe... \n");
                break;
            }
        }

        CFBASE = mem.GetBaseDaddy("crossfire.exe");
        CFSHELL = mem.GetBaseDaddy("CShell_x64.dll");

        if (CFBASE && CFSHELL) {
            if (!mem.GetKeyboard()->InitKeyboard()) {
                dma_status = "DMA Connected But Keyboard Failed to Initialize";
            }
            else {
                dma_status = "DMA Connected";
                LOG("[!]Keyboard initialized...\n");
            }

            dma_success.store(true);

            UpdateOffsets(mem);
            while (update_status == "Updating Offsets..." || update_status == "Idle") {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            if (update_status == "Offsets Updated")
            {
                RUNCACHE = true;
                g_cacheManager.StartUpdateThread(mem);
            }
             
          

            LOG("[X] Cheats not working? Try Forcing Update!\n");
            LOG("[X] Cheats still not working? Restart your PC, sometimes DMA Device just stops reading!\n");
            LOG("[!] If that fails, then sigs/structs are out of Date!\n");

             
             
            VMMDLL_ConfigSet(mem.vHandle, VMMDLL_OPT_REFRESH_ALL, 1);
            VMMDLL_ConfigSet(mem.vHandle, VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_PARTIAL, 300);
            VMMDLL_ConfigSet(mem.vHandle, VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_TOTAL, 1200);
        }
        else {
            if (mem.vHandle) {
                VMMDLL_Close(mem.vHandle);
                mem.vHandle = nullptr;
            }
            Memory::DMA_INITIALIZED = FALSE;
            Memory::PROCESS_INITIALIZED = FALSE;
            dma_status = "Connection Failed";
            dma_success.store(false);
        }

        initializing_dma = false;
        }).detach();
}

 
void attempt_kmbox_connection() {
    int kmResult = kmBoxBMgr.init();

    switch (kmResult) {
    case -1:
        kmbox_status = "KMBOX/MAKCU Failed!";
        return;
    case -2:
        kmbox_status = "KMBOX/MAKCU Port Not Open!";
        return;
    case 0:
        break; 
    default:
        kmbox_status = "Initializing...";
        return;
    }

    HANDLE hSerial = kmBoxBMgr.getSerialHandle();
    if (hSerial == INVALID_HANDLE_VALUE) {
        kmbox_status = "KMBOX/MAKCU Failed!";
        return;
    }

    std::string MAKCUCHECK = "km.version()\r";
    send_command(hSerial, MAKCUCHECK);

    char buffer[256];
    DWORD bytesRead;
    std::string response;

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;

            if (response.find(">>>") != std::string::npos) {
                break;
            }
        }
        else {
            break;
        }
    }

    
    if (response.empty()) {
        kmbox_status = "KMBOX/MAKCU Failed!";
        kmbox_connected = false;
    }
    else if (response.find("km.MAKCU") != std::string::npos) {
        kmbox_status = "MAKCU Connected";
        kmbox_connected = true;
    }
    else {
        kmbox_status = "KMBOX Connected";
        kmbox_connected = true;
    }
}


ImVec2 ImRotationCenter()
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);  

    const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);  
}
void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
{
    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
}
 


void set_mouse_passthrough(HWND hwnd) {

   
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (overlayMode == 0) {
        if (showMenu) {
           
            SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_LAYERED);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else {
           
             
            //SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_LAYERED);
           SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);

        }
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_LAYERED);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
     
    }

    if (allow_clicks)
    {
          style &= ~WS_EX_NOACTIVATE;
        SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_LAYERED);
        //  SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE);
    }
     



    // SetWindowPos(hwndMenu, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

 
/*if (!VMProtectIsProtected())
   {

       Sleep(500);
       exit(0);
   }*/

void VerifyLogin()
{

    if (VMProtectIsDebuggerPresent(true)) {
        std::cerr << "Unwanted Application Detected.. Exiting.." << std::endl;
        Sleep(500);
        exit(0);
    }

    if (!VMProtectIsValidImageCRC()) {
        std::cerr << "Corrupted Application...Exiting..." << std::endl;
        Sleep(500);
        exit(0);
    }

    std::string name = skCrypt("CrossfireManager").decrypt();
    std::string ownerid = skCrypt("6oVNWtNqyj").decrypt();
    //std::string secret = skCrypt("c50a572025029f144b3d3d6f3c3e93f0d93f591ba172fbc760ca470437e34911").decrypt();
    std::string version = skCrypt("1.0").decrypt();
    std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt();
    std::string path = skCrypt("").decrypt();

    static api KeyAuthApp(name, ownerid, version, url, path);
    KeyAuthApp.init();

    std::string key;
    std::cout << skCrypt("\nEnter license: ") << std::flush;
    std::cin >> key;

    g_sanitizedKey = sanitizeLicense(key);
    if (g_sanitizedKey.empty()) {
        std::cerr << "Invalid license format. Exiting..." << std::endl;
        Sleep(3500);
        exit(0);
    }


    KeyAuthApp.license(g_sanitizedKey);
    if (!KeyAuthApp.response.success) {
        std::cerr << "\n" << KeyAuthApp.response.message << std::endl;
        Sleep(3500);
        exit(0);
    }


    


    KeyAuthApp.log(g_sanitizedKey + " logged in");

    if (!KeyAuthApp.user_data.subscriptions.empty()) {
        auto string_to_timet = [](const std::string& str) -> time_t {
            try { return static_cast<time_t>(std::stoll(str)); }
            catch (...) { return 0; }
            };

        auto timet_to_tm = [](time_t t) -> tm* {
            return localtime(&t);
            };

        auto tm_to_readable_time = [](tm* timeinfo) -> std::string {
            char buffer[80];
            if (!timeinfo) return "Invalid Time";
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            return std::string(buffer);
            };

        std::string expiryStr = KeyAuthApp.user_data.subscriptions[0].expiry;
        time_t expiryTime = string_to_timet(expiryStr);
        tm* expiryTm = timet_to_tm(expiryTime);
        std::string readableExpiry = tm_to_readable_time(expiryTm);
        std::cout << "Subscription expiry: " << readableExpiry << std::endl;
    }
    else {
        std::cout << "No Subscription Found... Odd.." << std::endl;
    }

    bgThread = std::jthread([](std::stop_token st) {
        bool disconnectCondition = false;
        bool banCondition = false;
        std::string disconnectMessage;
        std::string banMessage;
        while (!st.stop_requested()) {

            std::this_thread::sleep_for(std::chrono::seconds(30));

            KeyAuthApp.check();
            if (!KeyAuthApp.response.success) {
                disconnectCondition = true;
                disconnectMessage = KeyAuthApp.response.message;
                break;
            }
            KeyAuthApp.license(g_sanitizedKey);
            if (!KeyAuthApp.response.success) {
                disconnectCondition = true;
                disconnectMessage = KeyAuthApp.response.message;
                break;
            }

            

           
            if (KeyAuthApp.user_data.subscriptions.empty()) {
                disconnectCondition = true;
                disconnectMessage = KeyAuthApp.response.message;
                break;
            }

           /**/ if (IsDebuggerPresent()) {
                banCondition = true;
                banMessage = "Account has been Disconnected.";
                KeyAuthApp.log( " got banned for Debugger");
                break;
                
            }

            if (VMProtectIsDebuggerPresent(true)) {
                banCondition = true;
                banMessage = "Account has been Disconnected.";
                KeyAuthApp.log(" got banned for Debugger");
                break;
            }

            if (!VMProtectIsValidImageCRC()) {
                banCondition = true;
                banMessage = "Account has been Disconnected.";
                KeyAuthApp.log(" got banned for modifying exe");
                break;
            }

            
        }

        if (banCondition) {
            std::string hwid = KeyAuthApp.user_data.hwid;
            
            KeyAuthApp.ban(banMessage);
            std::cerr << banMessage << std::endl;
            KeyAuthApp.logout();
            Sleep(1500);
            exit(0);
        }
        if (disconnectCondition) {
            std::cerr << disconnectMessage << std::endl;
            KeyAuthApp.logout();
            Sleep(1500);
            exit(0);
        }
        });

    if (!VMProtectIsValidImageCRC()) {
        std::string hwid = KeyAuthApp.user_data.hwid;
        KeyAuthApp.logout();
        std::cerr << "Invalid image CRC. Exiting..." << std::endl;
        Sleep(1500);
        exit(0);
    }

    std::cout << "License validated. Loading cheat..." << std::endl;
}

     
 

 
void EnableGlassTransparency(HWND hwnd, bool enable) {
    if (enable) {
        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);
      
 
    }
    else {
        MARGINS margins = { 0, 0, 0, 0 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);
        LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE);

        
    }
}
void  outlined_text(const ImVec2& pos, ImU32 color, const char* text) {
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
void Particles()
{
    ImVec2 screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

    static ImVec2 partile_pos[100];
    static ImVec2 partile_target_pos[100];
    static float partile_speed[100];
    static float partile_radius[100];


    for (int i = 1; i < 50; i++)
    {
        if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
        {
            partile_pos[i].x = rand() % (int)screen_size.x + 1;
            partile_pos[i].y = 15.f;
            partile_speed[i] = 1 + rand() % 25;
            partile_radius[i] = rand() % 4;

            partile_target_pos[i].x = rand() % (int)screen_size.x;
            partile_target_pos[i].y = screen_size.y * 2;
        }

        partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], ImGui::GetIO().DeltaTime * (partile_speed[i] / 60));

        if (partile_pos[i].y > screen_size.y)
        {
            partile_pos[i].x = 0;
            partile_pos[i].y = 0;
        }

        ImGui::GetWindowDrawList()->AddCircleFilled(partile_pos[i], partile_radius[i], ImColor(71, 226, 67, 255/2));
    }

}

 
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);

    if (GetMonitorInfo(hMonitor, &monitorInfo)) {
        if ( monitor_enum_state !=  target_monitor) {
             monitor_enum_state++;
            return TRUE;   
        }

         
        HWND hwnd = reinterpret_cast<HWND>(dwData);
        
         
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT);

      
        SetWindowPos(hwnd, HWND_TOPMOST,
            monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.top,
            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
            SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);

        return FALSE; 
    }
    return TRUE;
}


void set_monitor(int index, HWND hwnd) {
   target_monitor = index;
   monitor_enum_state = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(hwnd));

}



int main(int, char**)
{
    SetConsoleTitleA("MakimuraLoader");
    VerifyLogin();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Makimura", nullptr };
    ::RegisterClassExW(&wc);
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int primaryWidth = GetSystemMetrics(SM_CXSCREEN);
    int primaryHeight = GetSystemMetrics(SM_CYSCREEN);


   HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST |WS_EX_NOACTIVATE| WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"Makimura",
        WS_POPUP,
        150, 150, primaryWidth, primaryHeight,
        nullptr, nullptr, wc.hInstance, nullptr);

   
   EnableGlassTransparency(hwnd, true);
   set_monitor(0, hwnd);
    
    
   if (!CreateDeviceD3D(hwnd))
   {
       CleanupDeviceD3D();
       ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
       return 1;
   }

   ::ShowWindow(hwnd, SW_SHOWDEFAULT);
   ::UpdateWindow(hwnd);

    

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

    font::poppins_medium = io.Fonts->AddFontFromMemoryTTF(poppins_medium, sizeof(poppins_medium), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::poppins_medium_low = io.Fonts->AddFontFromMemoryTTF(poppins_medium, sizeof(poppins_medium), 15.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::tab_icon = io.Fonts->AddFontFromMemoryTTF(tabs_icons, sizeof(tabs_icons), 24.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::tahoma_bold = io.Fonts->AddFontFromMemoryTTF(tahoma_bold, sizeof(tahoma_bold), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::tahoma_bold2 = io.Fonts->AddFontFromMemoryTTF(tahoma_bold, sizeof(tahoma_bold), 28.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::chicons = io.Fonts->AddFontFromMemoryTTF(chicon, sizeof(chicon), 20.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());


    //if (image::bg == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDeviceMenu, background_image, sizeof(background_image), &info, pump, &image::bg, 0);

    if (image::bg == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, background_image, sizeof(background_image), &info, pump, &image::bg, 0);
    if (image::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), &info, pump, &image::logo, 0);
    if (image::logo_general == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo_general, sizeof(logo_general), &info, pump, &image::logo_general, 0);


    if (image::arrow == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, arrow, sizeof(arrow), &info, pump, &image::arrow, 0);
    if (image::bell_notify == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, bell_notify, sizeof(bell_notify), &info, pump, &image::bell_notify, 0);
    if (image::roll == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, roll, sizeof(roll), &info, pump, &image::roll, 0);


    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);



 

    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    bool done = false;
    while (!done)
    {

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            if (msg.hwnd == hwnd)
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }

            if (msg.message == WM_QUIT)
                done = true;
        }
        /*if (showMenu && GetForegroundWindow() != hwndMenu)
        {
           SetForegroundWindow(hwndMenu);
            SetFocus(hwndMenu);
        }*/

        if (mem.GetKeyboard()->IsKeyDown(0x2D) || GetAsyncKeyState(0x2D)) {
            showMenu = !showMenu;

           
            set_mouse_passthrough(hwnd);
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }

        if (done)
            break;
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }



        tab_size = ImLerp(tab_size, tab_opening ? 160.f : 0.f, ImGui::GetIO().DeltaTime * 12.f);
        arrow_roll = ImLerp(arrow_roll, tab_opening && (tab_size >= 159) ? 1.f : -1.f, ImGui::GetIO().DeltaTime * 12.f);
 

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        {


            ImGuiStyle* s = &ImGui::GetStyle();
            s->Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0.0f);
            s->WindowPadding = ImVec2(0, 0), s->WindowBorderSize = 0;
            s->ItemSpacing = ImVec2(20, 20);

            s->ScrollbarSize = 8.f;




            //ImGuiWindowFlags_NoDecoration

            ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);

            if (showMenu) {
                ImGui::Begin("IMGUI MENU", &showMenu, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground);
                {
                    
                    if (!showMenu)
                    {
                      
                        return 0;
                    }

                     
                    std::string displayKey = g_sanitizedKey;
                    size_t lastDash = displayKey.rfind('-');
                    if (lastDash != std::string::npos) {
                        size_t secondLastDash = displayKey.rfind('-', lastDash - 1);
                        if (secondLastDash != std::string::npos) {
                            displayKey = displayKey.substr(secondLastDash + 1);
                        }
                    }
                    const ImVec2& pos = ImGui::GetWindowPos();
                    const auto& p = ImGui::GetWindowPos();
                    const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;

                    ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(c::bg::size) + ImVec2(tab_size, 0), ImGui::GetColorU32(c::bg::background), c::bg::rounding);
                    ImGui::GetBackgroundDrawList()->AddRect(pos, pos + ImVec2(c::bg::size) + ImVec2(tab_size, 0), ImGui::GetColorU32(c::bg::outline_background), c::bg::rounding);

                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(c::accent_text_color));

                    ImGui::PushFont(font::tahoma_bold2); ImGui::RenderTextClipped(pos + ImVec2(60, 0) + spacing, pos + spacing + ImVec2(60, 60) + ImVec2(tab_size + (spacing.x / 2) - 30, 0), "CrossfireV1 ", NULL, NULL, ImVec2(0.5f, 0.5f), NULL); ImGui::PopFont();

                    ImGui::RenderTextClipped(pos + ImVec2(60 + spacing.x, c::bg::size.y - 60 * 2), pos + spacing + ImVec2(60, c::bg::size.y) + ImVec2(tab_size, 0), "User", NULL, NULL, ImVec2(0.0f, 0.43f), NULL);
                     
                    ImGui::RenderTextClipped(pos + ImVec2(60 + spacing.x, c::bg::size.y - 60 * 2), pos + spacing + ImVec2(60, c::bg::size.y) + ImVec2(tab_size, 0), displayKey.c_str(), NULL, NULL, ImVec2(0.0f, 0.57f), NULL);

                    ImGui::PushFont(font::tahoma_bold2); ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(250, 255, 255, 255)); ImGui::RenderTextClipped(pos + ImVec2(0, 0) + spacing, pos + spacing + ImVec2(60, 40) + ImVec2(tab_size + (spacing.x / 2) + 199, 0), "MakimuraDMA", NULL, NULL, ImVec2(1.f, 0.5f), NULL); ImGui::PopFont(); ImGui::PopStyleColor();

                    ImGui::GetBackgroundDrawList()->AddImage(image::logo, pos + ImVec2(10, 10), pos + ImVec2(10, 10), ImVec2(100, 100), ImVec2(100, 100), ImColor(255, 255, 255, 255));


                    //  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(90, 93, 100,255)); ImGui::RenderTextClipped(pos + ImVec2(30, 20) + spacing, pos + spacing + ImVec2(60, 80) + ImVec2(tab_size + (spacing.x / 2) + 108, -20), "Welcome Back!", NULL, NULL, ImVec2(1.f, 0.5f), NULL); ImGui::PopStyleColor();

                    ImGui::SetCursorPos(ImVec2(385 + tab_size, -20) + (s->ItemSpacing * 2));
                    ImGui::BeginChild(" ", " ", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 60), false, ImGuiWindowFlags_NoScrollbar);

                    ImGui::PushFont(font::tab_icon);

                    float iconSpacing = ImGui::GetContentRegionAvail().x / 7.0f;
                    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.3f);

                    ImVec2 textPos = ImGui::GetCursorScreenPos();
                    if (ImGui::Button("I", ImVec2(30, 30))) {
                        ShellExecute(0, 0, L"https://discord.com/users/makimura.dev", 0, 0, SW_SHOW);

                    }
                    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), "I");
                    ImGui::SameLine(0, iconSpacing);

                    textPos = ImGui::GetCursorScreenPos();
                    if (ImGui::Button("H", ImVec2(30, 30))) {
                        ShellExecute(0, 0, L"https://www.unknowncheats.me/forum/index.php", 0, 0, SW_SHOW);
                    }
                    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), "H");
                    ImGui::SameLine(0, iconSpacing);

                    textPos = ImGui::GetCursorScreenPos();
                    if (ImGui::Button("G", ImVec2(30, 30))) {
                        ShellExecute(0, 0, L"https://github.com/TheRyoSaeba", 0, 0, SW_SHOW);
                    }
                    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), "G");

                    ImGui::PopFont();




                    ImGui::EndChild();
                    ImGui::PopStyleColor(1);


                    const char* nametab_array1[6] = { "E", "D", "A", "B", "C","F" };

                    const char* nametab_array[6] = { "Aimbot", "Visuals", "World", "Configs", "Settings", "Miscallenous " };
                    const char* nametab_hint_array[6] = { "Rcs,Trigger", "ESP,Radar", "World Modifications", "Save your settings", "DMA Setup/Update","More Cheats" };


                    static int tabs = 0;

                    ImGui::SetCursorPos(ImVec2(spacing.x + (60 - 22) / 2, 60 + (spacing.y * 2) + 22));
                    ImGui::BeginGroup();
                    {
                        for (int i = 0; i < sizeof(nametab_array1) / sizeof(nametab_array1[0]); i++)
                            if (ImGui::Tab(i == tabs, nametab_array1[i], nametab_array[i], nametab_hint_array[i], ImVec2(35 + tab_size, 35))) tabs = i;
                    }
                    ImGui::EndGroup();

                    ImGui::SetCursorPos(ImVec2(8, 9) + spacing);

                    ImRotateStart();
                    if (ImGui::CustomButton(1, image::roll, ImVec2(35, 35), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::accent_color))) tab_opening = !tab_opening;
                    ImRotateEnd(1.57f * arrow_roll);

                    //ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(0, -20 + spacing.y) + spacing, pos + spacing + ImVec2(60, c::bg::size.y - 60 - spacing.y * 3) + ImVec2(tab_size, 0), ImGui::GetColorU32(c::child::background), c::child::rounding);
                    //ImGui::GetBackgroundDrawList()->AddRect(pos + ImVec2(0, -20 + spacing.y) + spacing, pos + spacing + ImVec2(60, c::bg::size.y - 60 - spacing.y * 3) + ImVec2(tab_size, 0), ImGui::GetColorU32(c::child::outline_background), c::child::rounding);

                    ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(0, c::bg::size.y - 60 - spacing.y * 2) + spacing, pos + spacing + ImVec2(60, c::bg::size.y - spacing.y * 2) + ImVec2(tab_size, 0), ImGui::GetColorU32(c::child::background), c::child::rounding);
                    ImGui::GetBackgroundDrawList()->AddRect(pos + ImVec2(0, c::bg::size.y - 60 - spacing.y * 2) + spacing, pos + spacing + ImVec2(60, c::bg::size.y - spacing.y * 2) + ImVec2(tab_size, 0), ImGui::GetColorU32(c::child::outline_background), c::child::rounding);

                    ImGui::GetWindowDrawList()->AddImage(image::logo, pos + ImVec2(0, c::bg::size.y - 60 - spacing.y * 2) + spacing + ImVec2(12, 12), pos + spacing + ImVec2(60, c::bg::size.y - spacing.y * 2) - ImVec2(12, 12), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));

                    ImGui::GetWindowDrawList()->AddCircleFilled(pos + ImVec2(63, c::bg::size.y - (spacing.y * 2) + 3), 10.f, ImGui::GetColorU32(c::child::background), 100.f);
                    ImGui::GetWindowDrawList()->AddCircleFilled(pos + ImVec2(63, c::bg::size.y - (spacing.y * 2) + 3), 6.f, ImColor(0, 255, 0, 255), 100.f);

                    // Particles();


                    static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

                    tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
                    tab_add = ImClamp(tab_add + (std::round(350.f) * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                    if (tab_alpha == 0.f && tab_add == 0.f) active_tab = tabs;

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * s->Alpha);

                     
                    
                         

                    if (tabs == 0) {   
                        float panelWidth = 300.f;  
                        float panelHeight = 470.0f;  
                        
                        
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.53f, 0.81f, 0.98f, 0.25f));
                        ImGui::SetCursorPos(ImVec2(60 + tab_size, 90) + (s->ItemSpacing * 2));
                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("E", "Aimbot ", ImVec2(panelWidth, panelHeight), true );
                            {
                                ImGui::PushItemWidth(-1);

                                ImGui::Text("Aimbot Settings");
                                ImGui::Separator();

                                
                                const char* hotkeys[] = { "LMB", "RMB",  "Alt", "Shift", "Ctrl" };
                                ImGui::Combo("Hotkey", &firstHotkey, hotkeys, IM_ARRAYSIZE(hotkeys));
                                

                                ImGui::Separator();
                                ImGui::Spacing();
                                ImGui::Checkbox("Enable Aimbot", &enableAimbot);
                                

                                ImGui::Separator();
                                 

                                 
                                float scaledAimFov = AimFov / 10.0f;
                                 

                               
                                ImGui::SliderFloat("Aimbot FOV", &scaledAimFov, 10.0f, 100.0f, "%.1f°");
                                AimFov = scaledAimFov * 10.0f;  

                                ImGui::Spacing();
                                ImGui::Separator();

                                
                                const char* smoothTypes[] = { "Linear", "Dynamic", "Koestep", "Trapezoid" };
                                 
                                ImGui::Combo("Smooth Type", &selectedSmoothType, smoothTypes, IM_ARRAYSIZE(smoothTypes));

                                ImGui::Spacing();
                                ImGui::Separator();

                               
                                ImGui::SliderFloat("Aim Distance", &MaxAimDistance, 10.0f, 1000.0f, "%.1f m");
                                 

                                ImGui::Spacing();

                                
                                ImGui::SliderFloat("Aim Speed", &AimSpeed, 0.01f, 0.1f, "%.2f");
                                 

                                ImGui::PopItemWidth();
                            }
                            ImGui::EndChild();
                            ImGui::PopStyleColor();
                        
                         
                    }
                    ImGui::EndGroup();
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    {
                         
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.53f, 0.81f, 0.98f, 0.25f));
                        ImGui::BeginChild("" , "Targeting", ImVec2(panelWidth, panelHeight), true);
                        {
                            ImGui::Text("Targeting Options");
                            ImGui::Separator();
                            ImGui::Spacing();

                             
                            static bool enableTriggerbot = false;
                            ImGui::Checkbox("Enable Triggerbot", &enableTriggerbot);

                            
                            static int triggerHotkey1 = 1;
                            static int triggerHotkey2 = 2;

                            ImGui::Text("First Hotkey");
                             

                            // Some toggles
                            static bool alwaysOn = false;
                            static bool onlyInScope = false;
                            ImGui::Checkbox("Always On", &alwaysOn);
                            ImGui::Checkbox("Only in Scope", &onlyInScope);

                            // Example extra settings
                            static float hitboxRadius = 0.10f;
                            ImGui::Text("Hitbox Radius");
                            ImGui::SliderFloat("##HitboxRadius", &hitboxRadius, 0.f, 2.f, "%.2f");

                            static float shootDelay = 1.0f;
                            ImGui::Text("Shoot Delay");
                            ImGui::SliderFloat("##ShootDelay", &shootDelay, 0.f, 2.f, "%.2f");
                        }
                        ImGui::EndChild();
                        ImGui::PopStyleColor();  
                    }
                    ImGui::EndGroup();
                    }

                    if (tabs == 1) {
                        ImGui::SetCursorPos(ImVec2(60 + tab_size, 60) + (s->ItemSpacing * 2));
                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("D", "MAIN", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 420));
                            {



                                ImGui::PushItemWidth(-1);

                                ImGui::Checkbox("Enable ESP", &Dcheckbox);

                                ImGui::Separator();


                               
                                const char* items[3]{ "Regular", "Corner", "Filled" };
                                ImGui::Combo("ESP Type", &esptype, items, IM_ARRAYSIZE(items), 3);


                                ImGui::Separator();

                                ImGui::Checkbox("ESP Lines", &showEspLines);


                                ImGui::Separator();



                                ImGui::MultiCombo("Flags", Flogs, Flogss, 4);


                                ImGui::Separator();

                                /* ImGui::Checkbox("Player Head", &Headcheckbox);
                                 ImGui::Checkbox("Health Bar", &Healthcheckbox);
                                 ImGui::Checkbox("Player Name", &Namecheckbox);
                                 ImGui::Checkbox("Distance", &Distancecheckbox);
                                 ImGui::Checkbox("Weapon", &weaponcheckbox); */
                                 //  ImGui::Checkbox("Bone", &Bonecheckbox);

                                ImGui::Checkbox("Bone", &Bonecheckbox);

                                ImGui::Separator();

                                ImGui::SliderInt("ESP Distance", &fov, 0, 1500);
                                

                                ImGui::Separator();

                                ImGui::Checkbox("Crosshair Notification", &crosshair_notify);


                                ImGui::Separator();

                                static char input[64] = { "" };
                                ImGui::InputTextWithHint("Exact Value", "Value..", input, 64, NULL);


                                ImGui::Separator();



                                


                                 


                                static int currentColorSelection = 0;


                                const char* colorChoices[] = { "Enemy","Traceline","Name","Bones", "Ally" };
                                ImGui::Combo("Color Editor", &currentColorSelection, colorChoices, IM_ARRAYSIZE(colorChoices));


                                static float colorTemp[4] = { 1.f, 1.f, 1.f, 1.f };


                                
                                switch (currentColorSelection) {
                                case 0: RGBAtoFloat4(g_EnemyColor, colorTemp); break;
                                case 1: RGBAtoFloat4(g_ESPLineColor, colorTemp); break;
                                case 2: RGBAtoFloat4(g_NameColor, colorTemp); break;
                                case 3: RGBAtoFloat4(g_HeadColor, colorTemp); break;
                                case 4: RGBAtoFloat4(g_AllyColor, colorTemp); break;
                                }

                               
                                if (ImGui::ColorEdit4("##ColorPicker", colorTemp,
                                    ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview |
                                    ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs))
                                {
                                    RGBA newColor = Float4toRGBA(colorTemp);

                                   
                                    switch (currentColorSelection) {
                                    case 0: g_EnemyColor = newColor; break;
                                    case 1: g_ESPLineColor = newColor; break;
                                    case 2: g_NameColor = newColor; break;
                                    case 3: g_HeadColor = newColor; break;
                                    case 4: g_AllyColor = newColor; break;
                                    }
                                }
                                ImGui::PopItemWidth();
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild(" ", "", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 420));
                            {
                                ImDrawList* draw = ImGui::GetWindowDrawList();
                                ImVec2 childPos = ImGui::GetWindowPos();
                                ImVec2 childSize = ImGui::GetWindowSize();
                                ImVec2 center(childPos.x + childSize.x * 0.5f,
                                    childPos.y + childSize.y * 0.5f);

                                float boxW = childSize.x * 0.5f;
                                float boxH = childSize.y * 0.6f;
                                ImVec2 boxTopLeft = ImVec2(childPos.x + (childSize.x - boxW) / 2, childPos.y + (childSize.y - boxH) / 2);
                                ImVec2 boxBottomRight = ImVec2(boxTopLeft.x + boxW, boxTopLeft.y + boxH);
                                ImU32 boxColor = IM_COL32(255, 0, 0, 255);

                                if (Dcheckbox || Flogs[0]) {

                                    
                                    switch (esptype) {
                                    case 0:  
                                        draw->AddRect(
                                            boxTopLeft,
                                            boxBottomRight,
                                            boxColor,
                                            0.0f, ImDrawFlags_Closed, boxtk
                                        );
                                        break;

                                    case 1:
                                        ESP::DrawCornerBox(
                                            static_cast<int>(boxTopLeft.x),
                                            static_cast<int>(boxTopLeft.y),
                                            static_cast<int>(boxW),
                                            static_cast<int>(boxH),
                                            boxtk,
                                            RGBA{ 255, 255, 255, 255 }
                                        );
                                        break;

                                    case 2:
                                        draw->AddRectFilled(
                                            boxTopLeft,
                                            boxBottomRight,
                                            IM_COL32(255, 0, 0, 100)  
                                        );
                                        draw->AddRect(
                                            boxTopLeft,
                                            boxBottomRight,
                                            boxColor,
                                            0.0f, ImDrawFlags_Closed, boxtk
                                        );
                                        break;
                                    }
                                }
                                if (Headcheckbox || Flogs[0])
                                {

                                    float headRadius = 5.0f + (hdtk * 3.0f);
                                    float headCenterY = center.y - (boxH / 2.f) - headRadius - 5.f;

                                    draw->AddCircle(
                                        ImVec2(center.x, headCenterY),
                                        headRadius,
                                        IM_COL32(255, 255, 255, 255),
                                        0,
                                        2.0f
                                    );
                                }

                                if (Healthcheckbox || Flogs[1])
                                {

                                    float barWidth = 4.0f + (hptk * 2.0f);
                                    float hpFraction = 0.75f;
                                    float filledHeight = boxH * hpFraction;

                                    float left = center.x - (boxW / 2.f) - 8.f;
                                    draw->AddRectFilled(
                                        ImVec2(left, center.y - boxH / 2 + (boxH - filledHeight)),
                                        ImVec2(left + barWidth, center.y + boxH / 2),
                                        IM_COL32(0, 255, 0, 255)
                                    );
                                }

                                if (Namecheckbox || Flogs[2])
                                {
                                    const char* name = "Enemy";
                                    ImVec2 textSize = ImGui::CalcTextSize(name);
                                    float textPosY = center.y - (boxH / 2.f) - textSize.y - 10.f;
                                    draw->AddText(ImVec2(center.x - textSize.x / 2.f, textPosY),
                                        IM_COL32(255, 255, 0, 255),
                                        name);
                                }

                                if (Flogs[3])
                                {

                                    char distanceText[32];
                                    float exampleDistance = 123.4f;
                                    sprintf(distanceText, "%.1fm", exampleDistance);


                                    ImVec2 textSize = ImGui::CalcTextSize(distanceText);
                                    float textY = center.y + boxH * 0.5f + 5.f;
                                    draw->AddText(ImVec2(center.x - textSize.x * 0.5f, textY),
                                        IM_COL32(255, 255, 255, 255),
                                        distanceText);
                                }






                                /*ImU32 allyColU32 = IM_COL32(g_AllyColor.R, g_AllyColor.G, g_AllyColor.B, g_AllyColor.A);
                                ImVec2 allyBoxPos = ImVec2(center.x + 10.f, center.y - boxH * 0.5f);
                                draw->AddRect(allyBoxPos,
                                    ImVec2(allyBoxPos.x + boxW, allyBoxPos.y + boxH),
                                    allyColU32,
                                    0.0f,
                                    0,
                                    2.0f);*/

                                    /*draw->AddText(ImVec2(allyBoxPos.x, allyBoxPos.y - 18.f),
                                        IM_COL32(255, 255, 255, 255),
                                        "Ally Box");*/


                                const char* previewLabel = "";
                                ImVec2 labelSize = ImGui::CalcTextSize(previewLabel);
                                draw->AddText(
                                    ImVec2(center.x - labelSize.x * 0.5f, center.y + boxH / 2.f + 8.f),
                                    IM_COL32(255, 255, 0, 255),
                                    previewLabel
                                );
                            }
                            ImGui::EndChild();
                            ImGui::BeginChild("C", "Others", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 80));
                            {
                                ImGui::SliderFloat("2D box thick", &boxtk, 1.f, 5.f, "%.1f");
                                ImGui::SliderFloat("Head Size", &hdtk, 1.f, 10.f, "%.1f");
                                ImGui::SliderFloat("Health Size", &hptk, 1.f, 5.f, "%.1f");
                                ImGui::SliderFloat("Bone thick", &bonetk, 1.f, 5.f, "%.1f");
                            }
                            ImGui::EndChild();
                            ImGui::SetCursorPos(ImVec2(60 + tab_size, 500) + (s->ItemSpacing * 2));
                            ImGui::BeginChild("A", "Team", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 80));
                            {
                                ImGui::Checkbox("Filter teams", &Filterteams);
                                ImGui::Checkbox("Enable Radar", &draw_radar);
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                    }

                    if (tabs == 3)
                    {

                       
                        allow_clicks = true;
                       set_mouse_passthrough(hwnd);
                        ImGui::SetCursorPos(ImVec2(30 + tab_size, 60) + (s->ItemSpacing * 2));
                        ImGui::BeginGroup();
                        {
                            
                            extern std::vector<std::string> GetCheatConfigList();
                            extern std::string getCheatConfigDir();
                            extern void SaveCheatConfig(const std::string & configName);
                            extern bool LoadCheatConfig(const std::string & configName);
                            ImVec2 childSize(650, 470);
                            
                            ScopedStyleColor sc(ImGuiCol_ChildBg, ImVec4(0, 0, 1, 1));
                            ImGui::BeginChild("B", "Config Menu", childSize, true);
                            {

                                
                                static char configName[64] = "default";
                                       
                                static std::vector<std::string> configList;
                                configList = GetCheatConfigList();
                                ImGui::PushItemWidth(300);   
                                ImGui::InputText("Config Name", configName, IM_ARRAYSIZE(configName));
                                
                                ImGui::Spacing();
                                float availableWidth = ImGui::GetContentRegionAvail().x;

                                float topButtonWidth = (availableWidth / 3.0f) - 10.0f;
                                float topButtonHeight = 50.0f;

                                if (ImGui::Button("Create/Save", ImVec2(topButtonWidth, topButtonHeight)))
                                {
                                    SaveCheatConfig(configName);
                                    LOG("[!] Config saved: %s\n", configName);
                                    configList = GetCheatConfigList();
                                     
                                }
                                ImGui::SameLine();
                                if (ImGui::Button("Folder", ImVec2(topButtonWidth, topButtonHeight)))
                                {
                                    std::string folderPath = getCheatConfigDir();
                                    ShellExecuteA(NULL, "open", folderPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                                }
                                ImGui::SameLine();
                                if (ImGui::Button("Refresh", ImVec2(topButtonWidth, topButtonHeight)))
                                {
                                    configList = GetCheatConfigList();
                                }

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();


                                availableWidth = ImGui::GetContentRegionAvail().x;
                                float configButtonWidth = (availableWidth / 3.0f) - 10.0f;
                                float configButtonHeight = 80.0f;

                                for (size_t i = 0; i < configList.size(); i++)
                                {
                                    if (ImGui::Button(configList[i].c_str(), ImVec2(configButtonWidth, configButtonHeight)))
                                    {
                                        if (LoadCheatConfig(configList[i]))
                                            LOG("[!] Config loaded: %s\n", configList[i].c_str());
                                        else
                                            LOG("[!] Failed to load config: %s\n", configList[i].c_str());
                                    }
                                   
                                    if ((i + 1) % 3 != 0)
                                        ImGui::SameLine();
                                }
                            }
                            ImGui::EndChild();



                        }
                        ImGui::EndGroup();

                    }
                    else
                    {
                         
                        allow_clicks = false;
                         
                        set_mouse_passthrough(hwnd);
                    }


                    if (tabs == 4) {
                        ImGui::SetCursorPos(ImVec2(60 + tab_size, 60) + (s->ItemSpacing * 2));
                        ImGui::BeginGroup();
                        {

                            ImGui::BeginChild("C", "DMA", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 420));
                            {
                                ImVec2 p = ImGui::GetWindowPos();
                                ImGui::GetWindowDrawList()->AddRectFilled(
                                    p,
                                    p + ImVec2(ImGui::GetWindowWidth(), 35),
                                    ImGui::GetColorU32(c::child::background),
                                    c::child::rounding
                                );
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                                ImGui::PushFont(font::tahoma_bold2);
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "DMA:");
                                ImGui::PopFont();
                                ImGui::Separator();


                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Status:");
                                ImGui::SameLine();
                                ImGui::TextColored(
                                    dma_status.find("Connected") != std::string::npos ?
                                    ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                    "%s", dma_status.c_str()
                                );

                                if (dma_success.load()) {
                                    ImGui::BeginDisabled();
                                    ImGui::Button("DMA Connected", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25));
                                    ImGui::EndDisabled();
                                }
                                else {
                                    if (initializing_dma) {
                                        ImGui::BeginDisabled();
                                        ImGui::Button("Initialize DMA", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25));
                                        ImGui::EndDisabled();
                                    }
                                    else {
                                        if (ImGui::Button("Initialize DMA", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {
                                            InitializeDMA(mem);



                                        }
                                    }
                                }


                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();



                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                                ImGui::PushFont(font::tahoma_bold2);
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "OFFSETS:");
                                ImGui::PopFont();
                                ImGui::Separator();


                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Status:");
                                ImGui::SameLine();
                                ImGui::TextColored(
                                    update_status == "Offsets Updated" ?
                                    ImVec4(0.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                                    "%s", update_status.c_str()
                                );

                                if (ImGui::Button("Force Update Config", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {
                                    ClearConfig();
                                    LOG("[!] Cleared the Config...\n");
                                     UpdateOffsets(mem);
                                      
                                     while (update_status == "Updating Offsets...") {
                                         std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                     }

                                     if (update_status == "Offsets Updated")
                                     {
                                         RUNCACHE = true;
                                         g_cacheManager.StartUpdateThread(mem);
                                     }
                                }
                            }
                        
                            ImGui::EndChild();

                            ImGui::BeginChild(" ", " Menu Overlay", ImVec2((c::bg::size.x - 80 - s->ItemSpacing.x * 3) / 2, 100), true);
                            {
                                const char* overlayModes[] = { "[1PC]Transparent", "[2PC]Fuser" };
                                if (ImGui::Combo("Overlay Mode", &overlayMode, overlayModes, IM_ARRAYSIZE(overlayModes))) {
                                    bool enableGlass = (overlayMode == 0);
                                    EnableGlassTransparency(hwnd, enableGlass);
                              

                                }

                                 


                                int monitorCount = GetSystemMetrics(SM_CMONITORS);
                                if (monitorCount > 1) {
                                    static int currentMonitor = 0;
                                    if (ImGui::Combo("Move Cheat", &currentMonitor, [](void* data, int idx, const char** out_text) {
                                        static char buffer[64];
                                        snprintf(buffer, sizeof(buffer), "Monitor %d", idx + 1);
                                        *out_text = buffer;
                                        return true;
                                        }, nullptr, monitorCount)) {
                                        set_monitor(currentMonitor, hwnd);

                                        set_mouse_passthrough(hwnd);

                                    }
                                }
                            }
                            ImGui::EndChild();  


                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();
                        ImGui::BeginGroup();
                        {

                            ImGui::BeginChild("C", "KMBOX", ImVec2((c::bg::size.x - 60 - s->ItemSpacing.x * 4) / 2, 420));
                            {
                                ImVec2 p = ImGui::GetWindowPos();
                                ImGui::GetWindowDrawList()->AddRectFilled(
                                    p,
                                    p + ImVec2(ImGui::GetWindowWidth(), 35),
                                    ImGui::GetColorU32(c::child::background),
                                    c::child::rounding
                                );
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                                ImGui::PushFont(font::tahoma_bold2);
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), " KMBOX:");
                                ImGui::PopFont();
                                ImGui::Separator();

                                if (!attempted_connection) {
                                    attempt_kmbox_connection();
                                    attempted_connection = true;
                                }
                                static std::string mouse_move_status = "Idle";
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Status:");
                                ImGui::SameLine();
                                ImGui::TextColored(
                                    (kmbox_status == "KMBOX Connected" || kmbox_status == "MAKCU Connected") ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) :
                                    (kmbox_status.find("Failed") != std::string::npos || kmbox_status.find("Port Not Open") != std::string::npos) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) :
                                    ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                                    "%s", kmbox_status.c_str()
                                );


                                if (!kmbox_connected) {
                                    if (ImGui::Button("KMBOX Connect", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {
                                        attempt_kmbox_connection();  
                                    }
                                }

                              
                                else {

                                    ImGui::BeginDisabled();
                                    ImGui::Button("KMBOX Connected", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25));
                                    ImGui::EndDisabled();
                                }
                                

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::PushFont(font::tahoma_bold2);
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Move Mouse:");
                                ImGui::PopFont();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::Text("Mouse Status:");
                                ImGui::SameLine();
                                ImGui::TextColored(
                                    mouse_move_status == "Moving..." ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) :  
                                    ImVec4(0.0f, 1.0f, 0.0f, 1.0f),  
                                    "%s", mouse_move_status.c_str()
                                );
                                if (ImGui::Button("Test Mouse Movement", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {
                                    if (kmbox_connected) {
                                        mouse_move_status = "Moving...";

                                       
                                        std::thread([]() {
                                            kmBoxBMgr.km_move_auto(150, 250, 2);
                                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                            mouse_move_status = "Done";
                                            }).detach();
                                    }
                                }
                            }
                            ImGui::EndChild();

                            ImGui::BeginChild(" ", "More Options", ImVec2((c::bg::size.x - 80 - s->ItemSpacing.x * 3) / 2, 100), true);
                            {

                                {
                                    if (ImGui::Button("Refresh Cheat", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {

                                        full_refresh();
                                    }

                                    if (ImGui::Button("Toggle Information", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {

                                        showInfoText = !showInfoText;
                                    }
                                    if (ImGui::Button("Exit", ImVec2(ImGui::GetContentRegionAvail().x - s->WindowPadding.x, 25))) {

                                        LOG("[!]See ya !\n");
                                        bgThread.request_stop();
                                        std::this_thread::sleep_for(std::chrono::milliseconds(300));
                                        exit(0);

                                    }

                                    /* ImGui::Checkbox("DMA AutoReconnect", &autoReconnect);
                                     ImGui::Separator();
                                     if (autoReconnect)
                                     {
                                         if (mem.vHandle)
                                         {

                                             g_cacheManager.StopUpdateThread();
                                             VMMDLL_Close(mem.vHandle);
                                             mem.vHandle = nullptr;

                                             Memory::DMA_INITIALIZED = FALSE;
                                             Memory::PROCESS_INITIALIZED = FALSE;
                                             initializing_dma = true;
                                             dma_status = "Not Initialized";


                                         }
                                     }*/
                                     // ImGui::Spacing();
                                     // ImGui::Separator();


                                    float comboWidth = 150.0f;
                                    ImGui::SetNextItemWidth(comboWidth);
                                    float windowWidth = ImGui::GetWindowWidth();
                                    ImGui::SetCursorPosX((windowWidth - comboWidth) * 0.5f);



                                }


                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                    }
                 }
                ImGui::PopStyleVar();



                ImGui::End();
                
            }
            if (showInfoText)
            {
                Particles();
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                ImDrawList* bgDraw = ImGui::GetBackgroundDrawList();
                float t = ImGui::GetTime();


                ImGui::PushFont(font::tahoma_bold2);
                const char* topText = "WELCOME BACK TO CROSSFIRE....";
                ImVec2 topTextSize = ImGui::CalcTextSize(topText);

                ImVec2 topTextPos = ImVec2((screenWidth - topTextSize.x) * 0.5f, 20.0f);
                outlined_text(topTextPos, IM_COL32(255, 255, 255, 255), topText);
                ImGui::PopFont();


                ImGui::PushFont(font::tahoma_bold);
                const char* bottomText = "MAKIMURA V1 CROSSFIRE DMA";
                ImVec2 bottomTextSize = ImGui::CalcTextSize(bottomText);

                ImVec2 bottomTextPos = ImVec2((screenWidth - bottomTextSize.x) * 0.5f,
                    topTextPos.y + topTextSize.y + 10);
                outlined_text(bottomTextPos, IM_COL32(255, 0, 0, 255), bottomText);
                ImGui::PopFont();


                ImGui::PushFont(font::tahoma_bold2);
                const char* menuText = "PRESS INSERT TO SHOW/HIDE MENU";
                ImVec2 menuTextSize = ImGui::CalcTextSize(menuText);

                float bounce = sin(t * 3.0f) * 5.0f;
                ImVec2 menuTextPos = ImVec2((screenWidth - menuTextSize.x) * 0.5f,
                    screenHeight - menuTextSize.y - 20.0f + bounce);

                float animAlpha = (sin(t * 3.0f) + 1.0f) * 0.5f;  // oscillates between 0 and 1.
                outlined_text(menuTextPos, IM_COL32(255, 255, 255, (int)(animAlpha * 255)), menuText);
                ImGui::PopFont();


                ImGui::PushFont(font::tahoma_bold);
                char fpsBuffer[32];
                sprintf(fpsBuffer, "FPS: %d", (int)ImGui::GetIO().Framerate);
                ImVec2 fpsTextSize = ImGui::CalcTextSize(fpsBuffer);
                ImVec2 fpsTextPos = ImVec2(screenWidth - fpsTextSize.x - 20.0f, 20.0f);
                outlined_text(fpsTextPos, IM_COL32(255, 255, 255, 255), fpsBuffer);
                ImGui::PopFont();
            }
             

            if (draw_radar && RUNCACHE)
            {
                try
                {
                    g_cacheManager.StartUpdateThread(mem);
                    const ImGuiViewport* viewport = ImGui::GetMainViewport();

                    auto snapshot = g_cacheManager.GetSnapshot();
                    float radarSizeFactor = viewport->Size.y * 0.2f;
                    ImVec2 radarPos = viewport->Pos + ImVec2(20, 20);
                    ImVec2 radarSize = ImVec2(radarSizeFactor, radarSizeFactor);
                    float radarRadius = 150.0f;
                    float radarMaxDistance = 7000.0f;
                    if (!snapshot)
                    {
                        LOG("Radar Crash!");
                        draw_radar = FALSE;
                    }
                    ImGui::SetNextWindowPos(radarPos, ImGuiCond_FirstUseEver);
                    ImGui::SetNextWindowSize(radarSize, ImGuiCond_FirstUseEver);
                    ImGui::SetNextWindowBgAlpha(0.0f);
                    if (ImGui::Begin("Radar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
                    {
                        radarSize = ImGui::GetWindowSize();
                        radarPos = ImGui::GetWindowPos();

                        ImVec2 radarCenter = radarSize * 0.5f;
                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        ImVec2 radarScreenPos = ImGui::GetWindowPos();


                        drawList->AddCircle(radarScreenPos + radarCenter, radarRadius, IM_COL32(255, 255, 255, 100), 64, 1.5f);

                        ImVec2 resizeHandleStart = radarScreenPos + ImVec2(radarSize.x - 12, radarSize.y - 12);
                        ImVec2 resizeHandleEnd = radarScreenPos + ImVec2(radarSize.x - 4, radarSize.y - 4);
                        drawList->AddLine(resizeHandleStart, resizeHandleEnd, IM_COL32(200, 200, 200, 180), 2.0f);


                        drawList->AddCircleFilled(radarScreenPos + radarCenter, 5.0f, IM_COL32(255, 255, 255, 255));
                        for (const auto& enemy : snapshot->enemies)
                        {
                            if (enemy.IsDead || enemy.Team == snapshot->localTeam)
                                continue;


                            float playerYaw = fmodf(snapshot->localYaw * (180.0f / M_PI), 360.0f); // Convert to degrees if necessary
                            if (playerYaw < 0) playerYaw += 360.0f;

                            D3DXVECTOR3 offset = enemy.AbsPos - snapshot->localAbsPos;

                            float cosYaw = cosf(-playerYaw * (M_PI / 180.0f));
                            float sinYaw = sinf(-playerYaw * (M_PI / 180.0f));
                            float rotatedX = (offset.x * cosYaw + offset.z * sinYaw);
                            float rotatedZ = (offset.z * cosYaw - offset.x * sinYaw);


                            float scaleFactor = radarRadius / radarMaxDistance;
                            ImVec2 enemyPos = radarCenter + ImVec2(rotatedX * scaleFactor, -rotatedZ * scaleFactor);
                            ImVec2 direction = enemyPos - radarCenter;
                            float distance = sqrtf(enemyPos.x * enemyPos.x + enemyPos.y * enemyPos.y);
                            if (distance > radarRadius - 10)
                            {
                                enemyPos = radarCenter + (enemyPos - radarCenter) * ((radarRadius - 10) / distance);
                            }


                            float enemyAngle = atan2f(rotatedX, rotatedZ);


                            drawList->AddCircleFilled(radarScreenPos + enemyPos, 5.0f, IM_COL32(255, 0, 0, 255));
                        }

                        ImGui::End();
                    }
                }
                catch (const std::exception& e)
                {
                    LOG("Radar Exception: %s", e.what());
                }
                catch (...)
                {
                    LOG("Radar Exception: Unknown error!");
                }

            }

            static auto lastUpdate = std::chrono::high_resolution_clock::now();
            auto now = std::chrono::high_resolution_clock::now();
            bool shouldDraw = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() >=2;

             

            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
             
            if (Dcheckbox || Bonecheckbox) {


                ImGui::Begin("Overlay", nullptr,
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

                // outlined_text(ImVec2(50, 50), IM_COL32(0, 255, 0, 255), "ESP : ON");
                float fps = ImGui::GetIO().Framerate;
                ImGui::Text("Framerate: %.1f FPS", fps);


                ImGui::End();
                
                if (shouldDraw && RUNCACHE) {
                    lastUpdate = now;
                    ImDrawList* draw = ImGui::GetForegroundDrawList();
                    g_cacheManager.StartUpdateThread(mem);
                    auto snapshot = g_cacheManager.GetSnapshot();
                    Render(mem, snapshot, draw);


                }
            }

        }
      
        ImGui::Render();

        const float clear_color_with_alpha[4] = {0.0,0.0,0.0,0.0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
   }


bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 144;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}




void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
    {
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);

        if (hWnd ==hWnd ) {
            if (g_pSwapChain) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
        }
         
         
        return 0;  
    }
    case WM_ACTIVATE:
    {
        if (LOWORD(wParam) == WA_INACTIVE) {

            if (overlayMode == 0) {
               
               allow_clicks = false;
                LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

                 
                SetWindowLong(hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

            }
            else
            {

                 

                  //  SetForegroundWindow(hWnd);
                    allow_clicks = false;
                    LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);


                    SetWindowLong(hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
                    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    //NG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
                    //Style &= ~WS_EX_LAYERED;
                    //SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);
                   
                    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                 
            }


        }
        else {

           allow_clicks = false;
        
             
           
            SetWindowLongA(hWnd, GWL_EXSTYLE, GetWindowLongA(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
           
        }
        return 0;
    }


    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
        }

        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
 

