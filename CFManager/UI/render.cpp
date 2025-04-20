#pragma once
#include "../ESP/ESP.h"
#include "render.h"
#include <Memory.h>
#include <thread>
#include "../Misc/Misc.h"
#include "../Aimbot/AimHelper.h"

#define ICON_GEAR u8"\b"

bool showMenu = true;

using namespace std::chrono_literals;

namespace MainThread {
    int imgui_menu(HWND hwnd);
}

namespace MainThread {

    int Render_Loop() {
        OverlayInitData overlay = SetupImGuiAndWindow();
        if (!overlay.hwnd)
            return -1;
        InitMenuFontsAndTextures();

        bool done = false;

        while (!done) {
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    done = true;
            }

            if (done)
                break;

            if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                    DXGI_FORMAT_UNKNOWN, 0);
                g_ResizeWidth = g_ResizeHeight = 0;
                CreateRenderTarget();
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            struct FrameContext {
                std::shared_ptr<ESP::Snapshot> snapshot;
                ImDrawList* drawList;
                bool inGame;

                FrameContext()
                    : snapshot(g_cacheManager.GetSnapshot()),
                    drawList(ImGui::GetForegroundDrawList()),
                    inGame(snapshot&& snapshot->m_clientShell.inGame()) {}
            };

            FrameContext ctx;

            if (ctx.snapshot) {
                if (ctx.inGame) {

                    if (Dcheckbox || Bonecheckbox) {
                        static auto nextESPFrame = std::chrono::steady_clock::now();
                        auto now = std::chrono::steady_clock::now();

                        if (now >= nextESPFrame) {
                            Render(mem, ctx.snapshot, ctx.drawList);
                            nextESPFrame = now + 2ms;
                        }
                    }


                    if (showFOVCircle) {
                        ESP::DrawFOVCircle(ctx.drawList, ctx.snapshot->drawPrim, AimFov,
                            IM_COL32(31, 255, 83, 100));
                        
                    }
                    uintptr_t Clntbase = reinterpret_cast<uintptr_t>(ctx.snapshot->m_clientShell.CPlayerClntBase);
                    mem.WriteValueInRange<float>(Clntbase + 0x700, Clntbase + 0x900, 4.000f);

                    if (draw_radar)
                        ESP::RenderRadar(mem, ctx.snapshot, ctx.drawList);

                    if (enableAimbot)
                    {
                        
                        DMA.AddTask(Aimbot::Run);
                    }

                    if (perWeaponConfig) {
                       WeaponConfigOverlay();
                    }

                        

                    if (memwrite)
                    {
                        if (camera_hacks)
                            DMA.AddTask(SetCameraPerspective, selectedPerspective, camOffset);
                        else

                            DMA.StopTask(typeid(SetCameraPerspective).name());

                        if (super_kill)
                            DMA.AddTask(SuperKill, mem);
                        else
                            DMA.StopTask(typeid(SuperKill).name());
                        if(fast_knives)
                       
                            DMA.AddTask(ShootThroughWall,mem);
                    else
                        DMA.StopTask(typeid(ShootThroughWall).name());

                    }
                  
                }
                

                if (showFPS)
                    ShowFPS(ctx.drawList);
            }

            // Manual refresh
            if (manual_refresh)
                std::thread([] { Memory::full_refresh(); }).detach();

            // ImGui menu
            
            MainThread::imgui_menu(overlay.hwnd);


            ImGui::EndFrame();
            ImGui::Render();

            const float clear_color_with_alpha[4] = { 0.0, 0.0, 0.0, 0.0 };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            g_pSwapChain->Present(1, 0);
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(overlay.hwnd);
        ::UnregisterClassW(overlay.wc.lpszClassName, overlay.wc.hInstance);

        return 0;
    }
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
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
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE) {
            SetForegroundWindow(hWnd);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
