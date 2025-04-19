#include "../Misc/Misc.h"
#include "render.h"
#include <Memory.h>
#include "../Authentication/auth.hpp"
#include "../region_header.h"
#include "../Config/globals.h"
#include "../Config/config.h"
 
namespace MainThread {



    int imgui_menu(HWND hwnd)
    {

       // DMA.AddTask(StartKeyCheck,  hwnd);
        
        

        bool isKeyPressed = (GetAsyncKeyState(showhidekey) & 0x8001) == 0x8001;  

        static bool previousKeyState = false;
        if (isKeyPressed && !previousKeyState) {
            showMenu = !showMenu;
            set_mouse_passthrough(hwnd);
        }
        previousKeyState = isKeyPressed;
        ImGuiStyle* style = &ImGui::GetStyle();
        

        c::accent = { color[0], color[1], color[2], 1.f };
        c::bg::outline = c::accent;
       
        style->WindowPadding = ImVec2(0, 0);
        style->ItemSpacing = ImVec2(20, 20);
        style->WindowBorderSize = 0;
        style->ScrollbarSize = 10.f;


        if (showMenu) {
            ImGui::Begin("IMGUI MENU", &showMenu,
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoBackground);
            {
                const ImVec2& pos = ImGui::GetWindowPos();
                const ImVec2& region = ImGui::GetContentRegionMax();
                const ImVec2& spacing = style->ItemSpacing;


                ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(region), ImGui::GetColorU32(c::bg::background), c::bg::rounding);
                ImGui::GetBackgroundDrawList()->AddImageRounded(texture::roughness, pos, pos + ImVec2(region), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::bg::roughness), c::bg::rounding);
                ImGui::PushFont(font::icomoon[30]);

                const char* logoIcon = "L";
                const char* logoText = "MakimuraDMA";

                ImVec2 iconSize = ImGui::CalcTextSize(logoIcon);
                ImVec2 textSize = ImGui::CalcTextSize(logoText);
                ImVec2 totalSize = iconSize + ImVec2(12.0f, 0.0f) + textSize;

                ImVec2 centerPos = pos + (ImVec2(200, 50) - totalSize) * 0.3f + ImVec2(200, 50) * 0.3f;
                ImGui::GetWindowDrawList()->AddText(centerPos, ImGui::GetColorU32(c::accent), logoIcon);
                ImGui::PushFont(font::tahoma_bold);

                ImGui::GetWindowDrawList()->AddText(centerPos + ImVec2(iconSize.x + 15.0f, 0.0f), ImGui::GetColorU32(c::accent), logoText);

                ImGui::PopFont();
                ImGui::PopFont();
                 
                const char* rightLogoIcon = "R";
                const char* rightLogoText = "CrossfireManager";
                ImVec2 rIconSize = ImGui::CalcTextSize(rightLogoIcon);
                ImVec2 rTextSize = ImGui::CalcTextSize(rightLogoText);
                ImVec2 rTotalSize = rIconSize + ImVec2(12.0f, 0.0f) + rTextSize;

                float headerCenterY = pos.y + 25.0f;  // Center of 50px header
                float iconVerticalCenter = headerCenterY - rIconSize.y / 2;


                ImVec2 rIconPos(pos.x + region.x - rTotalSize.x - 20.0f, iconVerticalCenter);
                ImVec2 rTextPos = rIconPos + ImVec2(rIconSize.x + 15.0f, 0.0f);

                // Draw right side elements
                ImGui::PushFont(font::icomoon[30]);
                ImGui::GetWindowDrawList()->AddText(rIconPos, ImGui::GetColorU32(c::accent), rightLogoIcon);
                ImGui::PopFont();

                ImGui::PushFont(font::tahoma_bold);
                ImGui::GetWindowDrawList()->AddText(rTextPos, ImGui::GetColorU32(c::accent), rightLogoText);
                ImGui::PopFont();

                //ImGui::GetWindowDrawList()->AddText(pos + ImVec2((50 - ImGui::CalcTextSize("L").y) / 2, (50 - ImGui::CalcTextSize("L").y) / 2), ImGui::GetColorU32(c::accent), "L");

                ImGui::PushFont(font::icomoon[21]);
                //drawtablogo
                //ImGui::GetWindowDrawList()->AddText(pos + ImVec2(200 + (50 - ImGui::CalcTextSize("a").y) / 2, (50 - ImGui::CalcTextSize("a").y) / 2), ImGui::GetColorU32(c::accent), "a");


                //ImGui::GetWindowDrawList()->AddText(pos + ImVec2(region.x - (50 + ImGui::CalcTextSize("b").y) / 2, (50 - ImGui::CalcTextSize("b").y) / 2), ImGui::GetColorU32(c::text::text), "b");

                ImGui::PopFont();

                // ImGui::GetWindowDrawList()->AddText(pos + ImVec2(200 + 45, (50 - ImGui::CalcTextSize("Aimb0t").y) / 2 + 1), ImGui::GetColorU32(c::text::text_active), "Aimb0t");
                 //ImGui::GetWindowDrawList()->AddText(pos + ImVec2(45, (50 - ImGui::CalcTextSize("GEAR5.AC").y) / 2 + 1), ImGui::GetColorU32(c::text::text_active), "GEAR5.AC");

                ImGui::GetBackgroundDrawList()->AddLine(pos + ImVec2(200, 0), pos + ImVec2(200, region.y), ImGui::GetColorU32(c::bg::outline), 1.f);
                ImGui::GetBackgroundDrawList()->AddLine(pos + ImVec2(0, 50), pos + ImVec2(region.x, 50), ImGui::GetColorU32(c::bg::outline), 1.f);

                ImGui::SetCursorPos(ImVec2(0, 50 + spacing.y));
                ImGui::BeginGroup();//define tabs
                {
                    //dqsg
                    if (ImGui::Tabs(0 == tabs, "a", "Aimbot", "Aimbot & Triggerbot", ImVec2(200, 70))) tabs = 0;

                    if (ImGui::Tabs(1 == tabs, "b", "Visuals", " ESP & Radar ", ImVec2(200, 70))) tabs = 1;

                    if (ImGui::Tabs(2 == tabs, "c", "World", "World & More", ImVec2(200, 70))) tabs = 2;

                    if (ImGui::Tabs(3 == tabs, "e", "Settings", "DMA,KMBOX & More", ImVec2(200, 70))) tabs = 3;
                }
                ImGui::EndGroup();

                ImGui::SetCursorPos(ImVec2(200 + spacing.x, 0));
                if (tabs == 0) { //aimbot tab
                    static int tabsHor = 0;
                    ImGui::BeginGroup();
                    {
                        //settings
                        if (ImGui::TabsHor(0 == tabsHor, "c", "general", "Aimbot General", ImVec2(100, 50))) tabsHor = 0;
                        ImGui::SameLine();
                        //finished
                        if (ImGui::TabsHor(1 == tabsHor, "b", "Class Config", "Just a description of this tab", ImVec2(100, 50))) tabsHor = 1;
                        ImGui::SameLine();
                        //finished
                        if (ImGui::TabsHor(2 == tabsHor, "c", "triggerbot", "Just a description of this tab", ImVec2(100, 50))) tabsHor = 2;
                        ImGui::SameLine();
                        //finished
                     
                        //finished
                    }
                    ImGui::EndGroup();

                    ImGui::SetCursorPos(ImVec2(200, 50) + spacing);
                    ImGui::BeginChild("Child", ImVec2(region.x - 200, region.y - 50) - spacing);
                    switch (tabsHor)
                    {
                    case 0:


                    {
                        ImGui::BeginGroup();
                        {
                            ImGui::CustomBeginChild("Aimbot Options", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {

                                ImGui::Checkbox("Enable Aimbot", &enableAimbot);

                                ImGui::SliderFloat("Aimbot FOV", &AimFov, 10.0f, 100.0f, "%.1f deg");

                                ImGui::Spacing();

                                ImGui::Keybind("AimKey", &aimkey, &keymode, true);

                               

                            }
                            ImGui::CustomEndChild();
 
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {

                            float w = (region.x - 200 - spacing.x * 3) * 0.5f;
                            ImGui::CustomBeginChild("Target Selection", ImVec2(w, 0));
                            {

                                const char* aimPosOptions[] = { "Head", "Body", "Closest" };
                                ImGui::Combo("Aim Position", &AimPosition, aimPosOptions, IM_ARRAYSIZE(aimPosOptions));

                                ImGui::SliderFloat("Target Distance", &MaxAimDistance, 10.0f, 500.0f, "%.1f m", ImGuiSliderFlags_AlwaysClamp);

                                ImGui::SliderInt("Smoothing", &smoothing, 1,  10, "%d", ImGuiSliderFlags_AlwaysClamp);

                                 const char* targetoptions[] = { "Screen", "World", "Combined" };
                                 ImGui::Combo("Prioritize", &TargetSwitch, targetoptions, IM_ARRAYSIZE(targetoptions));
                         
                              
                            }
                            ImGui::CustomEndChild();

                            

                         
                        }
                        ImGui::EndGroup();

                        ImGui::Spacing();

                    }

                    break;

                    case 1:
                    {
                        ImGui::BeginGroup();
                        {
                            ImGui::CustomBeginChild("Class Configs", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {
                                static bool checkbook = 0;

                                ImGui::Checkbox("Enable Class Configs", &perWeaponConfig);

                              //  ImGui::Combo("Select Class", &select_combo, items, IM_ARRAYSIZE(items), ARRAYSIZE(items));

                            }
                            ImGui::CustomEndChild();
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {
                            ImGui::CustomBeginChild("empty", ImVec2(region.x - (200 + spacing.x * 3), 0) / 2);
                            {


                                

                            }
                            ImGui::CustomEndChild();
                        }
                        ImGui::EndGroup();
                        break;
                    }
                    case 2:
                        break;
                    case 3:
                        break;

                    default:
                        break;

                    }
                    ImGui::EndChild();
                }
                if (tabs == 1) {
                    static int visualstabsHor = 0;
                    ImGui::BeginGroup();
                    {

                        if (ImGui::TabsHor(0 == visualstabsHor, "b", "ESP", "ESP Setup", ImVec2(100, 50))) visualstabsHor = 0;
                        ImGui::SameLine();

                        if (ImGui::TabsHor(1 == visualstabsHor, "g", "Radar", "Radar Setup", ImVec2(100, 50))) visualstabsHor = 1;
                        ImGui::SameLine();

                    }
                    ImGui::EndGroup();

                    ImGui::SetCursorPos(ImVec2(200, 50) + spacing);
                    ImGui::BeginChild("Child", ImVec2(region.x - 200, region.y - 50) - spacing);
                    switch (visualstabsHor)
                    {
                    case 0:


                    {
                        ImGui::BeginGroup();
                        {
                            ImGui::CustomBeginChild("Visuals", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {

                                ImGui::Checkbox("Enable ESP", &Dcheckbox);
                                ImGui::Checkbox("Bone", &Bonecheckbox);
                                ImGui::Checkbox("ESP Lines", &showEspLines);
                                ImGui::Checkbox("Crosshair Notification", &crosshair_notify);

                                ImGui::Checkbox("FOV", &showFOVCircle);
                                ImGui::Checkbox("Filter teams", &Filterteams);
                            }
                            ImGui::CustomEndChild();

                            ImGui::CustomBeginChild("Selection ESP", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {


                                ImGui::MultiCombo("Flags", Flogs, Flogss, 4);

                                const char* items[4]{ "Regular", "Corner", "Filled","3D"};
                                ImGui::Combo("ESP Type", &esptype, items, IM_ARRAYSIZE(items));



                            }
                            ImGui::CustomEndChild();




                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {

                            ImGui::CustomBeginChild("Filters", ImVec2(region.x - (200 + spacing.x * 3), 0) / 2);
                            {
                                ImGui::SliderInt("ESP Distance", &fov, 0, 1000, "%d", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("2D box thick", &boxtk, 1.f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Head Size", &hdtk, 1.f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Health Size", &hptk, 1.f, 5.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Bone thick", &bonetk, 1.f, 5.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);


                            }
                            ImGui::CustomEndChild();

                            ImGui::CustomBeginChild("Color Setup", ImVec2(region.x - (200 + spacing.x * 3), 0) / 2);
                            {

                                static int currentColorSelection = 0;
                                const char* colorChoices[] = { "Enemy", "Traceline", "Name", "Bones", "Ally" };
                                ImGui::Combo("Color Editor", &currentColorSelection, colorChoices, IM_ARRAYSIZE(colorChoices));
                                static float colorTemp[4] = { 1.f, 1.f, 1.f, 1.f };
                                if (currentColorSelection == 0) RGBAtoFloat4(g_EnemyColor, colorTemp);
                                else if (currentColorSelection == 1) RGBAtoFloat4(g_ESPLineColor, colorTemp);
                                else if (currentColorSelection == 2) RGBAtoFloat4(g_NameColor, colorTemp);
                                else if (currentColorSelection == 3) RGBAtoFloat4(g_HeadColor, colorTemp);
                                else if (currentColorSelection == 4) RGBAtoFloat4(g_AllyColor, colorTemp);
                                if (ImGui::ColorEdit4("Choose Color", colorTemp, picker_flags)) {
                                    RGBA newColor = Float4toRGBA(colorTemp);
                                    if (currentColorSelection == 0) g_EnemyColor = newColor;
                                    else if (currentColorSelection == 1) g_ESPLineColor = newColor;
                                    else if (currentColorSelection == 2) g_NameColor = newColor;
                                    else if (currentColorSelection == 3) g_HeadColor = newColor;
                                    else if (currentColorSelection == 4) g_AllyColor = newColor;
                                }

                            }
                            ImGui::CustomEndChild();







                        }
                        ImGui::EndGroup();

                    }

                    break;

                    case 1:
                    {
                        ImGui::BeginGroup();
                        {
                            ImGui::CustomBeginChild("Radar", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {
                                ImGui::Checkbox("Enable Radar", &draw_radar);
                                ImGui::Checkbox("Show as Arrows", &draw_enemies_as_arrows);
                                ImGui::Checkbox("Show Player Names", &draw_enemy_names);

                            }
                            ImGui::CustomEndChild();
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {
                            ImGui::CustomBeginChild("Radar Configuration", ImVec2(region.x - (200 + spacing.x * 3), 0) / 2);
                            {

                                ImGui::SliderFloat("Radar Distance (m)", &radarMaxDistance, 10.0f, 500.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Radar Size", &radar_size_factor, 0.1f, 0.4f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Background Opacity", &radar_bg_alpha, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Elevation Levels", &radar_elev_threshold, 1.0f, 5.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Radar Opacity", &radar_opacity, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Radar Zoom", &radar_zoom, 0.1f, 5.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

                            }
                            ImGui::CustomEndChild();
                        }
                        ImGui::EndGroup();
                        break;
                    }
                    default:
                        break;

                    }
                    ImGui::EndChild();
                }
                if (tabs == 2)
                {
                    static int misctabhor = 0;

                    ImGui::BeginGroup();
                    {
                       
                        if (ImGui::TabsHor(0 == misctabhor, "h", "Camera", "Camera Hacks", ImVec2(100, 50)))
                            misctabhor = 0;
                        ImGui::SameLine();
                        if (ImGui::TabsHor(1 == misctabhor, "e", "Weapon", "Weapon Hacks", ImVec2(100, 50)))
                            misctabhor = 1;


                        ImGui::SameLine();
                    }
                    ImGui::EndGroup();

                    ImGui::SetCursorPos(ImVec2(200, 50) + spacing);
                    ImGui::BeginChild("Child", ImVec2(region.x - 200, region.y - 50) - spacing);

                    switch (misctabhor)
                    {
                    case 0:
                    {
                        ImGui::BeginGroup();
                        {

                            ImGui::Checkbox("Enable Camera Hacks", &camera_hacks);

                            ImGui::CustomBeginChild("Camera (Memory Write Required)", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {
                                static int perspectiveIndex = 0;
                                const int perspectiveValues[] = { 1, 2, 3, 4, 6 };
                                const char* perspectiveLabels[] = { "First Person ", "Freecam", "Third Person", "TopDown", "Spectator" };

                                for (int i = 0; i < IM_ARRAYSIZE(perspectiveValues); ++i)
                                {
                                    if (selectedPerspective == perspectiveValues[i])
                                    {
                                        perspectiveIndex = i;
                                        break;
                                    }
                                }

                                if (ImGui::Combo("Camera Mode[Doesn't Work on some Modes]", &perspectiveIndex, perspectiveLabels, IM_ARRAYSIZE(perspectiveLabels)))
                                {
                                    selectedPerspective = perspectiveValues[perspectiveIndex];
                                    char toastText[64];
                                    snprintf(toastText, sizeof(toastText), "Camera Mode: %s", perspectiveLabels[perspectiveIndex]);
                                    ImGui::InsertNotification({ ImGuiToastType_Info, 4000, toastText });

                                }

                                
 
                                ImGui::SliderFloat("Camera X", &camOffset.x, -180.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Camera Y", &camOffset.y, -180.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                                ImGui::SliderFloat("Camera Z", &camOffset.z, -180.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
 

                            }
                            ImGui::CustomEndChild();

                            ImGui::CustomBeginChild("Spectate Players", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {
                                if (selectedPerspective >= 6)
                                {
                                    float buttonWidth = 100.0f;
                                    float spacingX = ImGui::GetStyle().ItemSpacing.x;

                                    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - (buttonWidth * 2 + spacingX)) * 0.5f);

                                    if (ImGui::Button("Previous", ImVec2(buttonWidth, 0)))
                                    {
                                        if (selectedPerspective > 6)
                                            selectedPerspective--;
                                    }

                                    ImGui::SameLine();

                                    if (ImGui::Button("Next", ImVec2(buttonWidth, 0)))
                                    {
                                        if (selectedPerspective < 9)
                                            selectedPerspective++;
                                    }

                                    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 200.0f) * 0.5f);
                                    ImGui::Text("Spectating Player Index: %d", selectedPerspective - 6);
                                }
                            }
                            ImGui::CustomEndChild();
                        }
                        ImGui::EndGroup();
                        break;
                    }

                    case 1:
                    {
                        ImGui::BeginGroup();
                        {

                            ImGui::CustomBeginChild("Weapon Hacks", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                            {
                               ImGui::Checkbox("Fast Knives [Memory Write Required]", &fast_knives);

                               
                               ImGui::Checkbox("SuperKill [Memory Write Required]", &super_kill);
                              
                            }
                            ImGui::CustomEndChild();
                        }
                        ImGui::EndGroup();
                        break;
                    }
                    default:
                        break;

                    }
                    ImGui::EndChild();
                }

                if (tabs == 3)
                {
                    static int settingsTabsHor = 0;

                    // Horizontal tabs at the top
                    ImGui::BeginGroup();
                    {
                        if (ImGui::TabsHor(settingsTabsHor == 0, "d", "DMA", "DMA Configuration", ImVec2(100, 50)))
                            settingsTabsHor = 0;
                        ImGui::SameLine();
                        if (ImGui::TabsHor(settingsTabsHor == 1, "e", "KMBOX", "KMBOX Input Settings", ImVec2(100, 50)))
                            settingsTabsHor = 1;
                        ImGui::SameLine();
                        if (ImGui::TabsHor(settingsTabsHor == 2, "n", "Configs", "Configs", ImVec2(100, 50)))
                            settingsTabsHor = 2;
                    }
                    ImGui::EndGroup();

                    // Main child area for the selected tab
                    ImGui::SetCursorPos(ImVec2(200, 50) + spacing);
                    ImGui::BeginChild("SettingsChild", ImVec2(region.x - 200, region.y - 50) - spacing);
                    {
                        switch (settingsTabsHor)
                        {
                        case 0: // DMA Tab
                        {
                            ImGui::BeginGroup();
                            {
                                ImGui::CustomBeginChild("Menu Options", ImVec2((region.x - (200 + spacing.x * 3)) / 2, 0));
                                {

                                    ImGui::Keybind("Menu Toggle", &showhidekey, &b, false);
                                    ImGui::ColorEdit4("Menu Colors", color, picker_flags);

                                    if (ImGui::Button("Display FPS",
                                        ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 30)))
                                    {
                                        showFPS = !showFPS;
                                    }
                                    if (ImGui::Checkbox("Enable Memory Write[Ban Risk]", &memwrite))
                                    {
                                        if (memwrite)
                                           
                                            ImGui::InsertNotification({ ImGuiToastType_Error, 1500,
                                            "Please Keep in mind Memory writes can be dangerous \n" });
                                    }
                                    if (ImGui::Button("Exit", ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 30))) {
                                        g_authMonitorThread.request_stop();
                                        DMA.StopAllTasks();
                                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                        TerminateProcess(GetCurrentProcess(), 0);
                                    }


                                    ImGui::CustomEndChild();
                                    ImGui::EndGroup();

                                    ImGui::SameLine();

                                    ImGui::BeginGroup();
                                    {
                                        ImGui::CustomBeginChild("Game Options",
                                            ImVec2(region.x - (200 + spacing.x * 3), 0) / 2);
                                        {
                                            const char* overlayModes[] = { "[1PC]Transparent", "[2PC]Fuser" };
                                            if (ImGui::Combo("Overlay Mode", &overlayMode, overlayModes,
                                                IM_ARRAYSIZE(overlayModes)))
                                            {
                                                bool enableGlass = (overlayMode == 0);
                                                EnableGlassTransparency(hwnd, enableGlass);
                                            }

 


                                            int monitorCount = GetSystemMetrics(SM_CMONITORS);
                                            if (monitorCount > 0)
                                            {
                                                static int currentMonitor = 0;
                                                if (ImGui::Combo("Switch Monitors", &currentMonitor,
                                                    [](void* data, int idx, const char** out_text)
                                                    {
                                                        static char buffer[64];
                                                        snprintf(buffer, sizeof(buffer), "Monitor %d", idx + 1);
                                                        *out_text = buffer;
                                                        return true;
                                                    },
                                                    nullptr, monitorCount))
                                                {
                                                    set_monitor(currentMonitor, hwnd);
                                                }
                                            }

                                             

                                            if (ImGui::Button("Refresh Cheat",
                                                ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 30)))
                                            {
                                                manual_refresh = true;

                                                ImGui::InsertNotification({ ImGuiToastType_Error, 1500,
                                                    "Queueing Refresh.. Please wait \n" });
                                                text_add++;
                                            }

                                        }
                                        ImGui::CustomEndChild();
                                    }
                                    ImGui::EndGroup();

                                    break;
                                }
                            }

                        case 1:
                        {
                            ImGui::BeginGroup();
                            {
                                ImGui::CustomBeginChild("KMBOX Options",
                                    ImVec2((region.x - (300 + spacing.x * 3)) / 2, 0));
                                {
                                    const float buttonHeight = 20.f;
                                    ImVec2 buttonSize(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 30);

                                    if (ImGui::Button("Connect KMBOX", buttonSize))
                                    {
                                        if (!kmbox_connected)
                                        {
                                            std::thread([]() { attempt_kmbox_connection(); }).detach();
                                            ImGui::InsertNotification({ ImGuiToastType_Info, 1500,
                                                "Attempting KMBOX connection..." });
                                        }
                                        else
                                        {
                                            ImGui::InsertNotification({ ImGuiToastType_Warning, 1500,
                                                "KMBOX is already connected." });
                                        }
                                    }

                                    if (ImGui::Button("Test KMBOX Mouse Movement", buttonSize))
                                    {
                                        if (kmbox_connected)
                                        {
                                            ImGui::InsertNotification({ ImGuiToastType_Info, 1500,
                                                "Moving Mouse..." });

                                            std::thread([]()
                                                {
                                                    kmBoxBMgr.km_move_auto(150, 250, 2);
                                                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                                    ImGui::InsertNotification({ ImGuiToastType_Success, 1500,
                                                        "Mouse Movement Completed." });
                                                }).detach();
                                        }
                                        else
                                        {
                                            ImGui::InsertNotification({ ImGuiToastType_Error, 1500,
                                                "KMBOX not connected!" });
                                        }
                                    }
                                }
                                ImGui::CustomEndChild();
                            }
                            ImGui::EndGroup();
                            break;
                        }

                        case 2:
                        {

                            static std::vector<std::string> configList;
                            static char configName[64] = "default";
                            static bool initialLoadDone = false;


                            if (configList.empty()) {
                                configList = GetCheatConfigList();


                                if (!initialLoadDone) {
                                    ImGui::InsertNotification({ ImGuiToastType_Success, 1500, "Loading configs" });
                                    text_add++;
                                    initialLoadDone = true;
                                }
                            }

                            ImGui::BeginGroup();
                            {
                                ImGui::CustomBeginChild("Configs", ImGui::GetContentRegionAvail());
                                {


                                    ImGui::InputTextEx("ConfigName",
                                        "Config Name",
                                        configName,
                                        IM_ARRAYSIZE(configName),
                                        ImVec2(300, 25),
                                        0);

                                    ImGui::Spacing();

                                    float availableWidth = ImGui::GetContentRegionAvail().x;
                                    float topButtonWidth = (availableWidth / 3.0f) - 10.0f;
                                    float topButtonHeight = 35.0f;

                                    if (ImGui::Button("Create/Save", ImVec2(topButtonWidth, topButtonHeight)))
                                    {
                                        SaveCheatConfig(configName);
                                        ImGui::InsertNotification({ ImGuiToastType_Success, 3500, "Config Saved : %s\n",configName });text_add++;
                                        configList = GetCheatConfigList();
                                    }

                                    ImGui::SameLine();

                                    if (ImGui::Button("Folder", ImVec2(topButtonWidth, topButtonHeight)))
                                    {
                                        std::string folderPath = getCheatConfigDir();
                                        ShellExecuteA(nullptr, "open", folderPath.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
                                    }

                                    ImGui::SameLine();

                                    if (ImGui::Button("Refresh", ImVec2(topButtonWidth, topButtonHeight)))
                                    {
                                        configList = GetCheatConfigList();
                                    }

                                    ImGui::Spacing();
                                    ImGui::Separator();
                                    ImGui::Spacing();

                                    float configButtonWidth = (availableWidth / 3.0f) - 10.0f;
                                    float configButtonHeight = 60.0f;

                                    if (!configList.empty())
                                    {
                                        for (size_t i = 0; i < configList.size(); i++)
                                        {
                                            if (ImGui::Button(configList[i].c_str(), ImVec2(configButtonWidth, configButtonHeight)))
                                            {
                                                if (LoadCheatConfig(configList[i]))
                                                {
                                                    std::string successMsg = "Config Loaded: " + configList[i];
                                                    ImGui::InsertNotification({ ImGuiToastType_Success, 3500, successMsg.c_str() });
                                                    text_add++;
                                                }
                                                else
                                                {
                                                    std::string failMsg = "Failed to load config: " + configList[i];
                                                    ImGui::InsertNotification({ ImGuiToastType_Error, 3500, failMsg.c_str() });
                                                }
                                            }

                                            if ((i + 1) % 3 != 0)
                                                ImGui::SameLine();
                                        }
                                    }
                                    else
                                    {
                                        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "No config files found.");
                                    }
                                }
                                ImGui::CustomEndChild();
                            }
                        }

                        ImGui::EndGroup();
                        break;
                        }
                        ImGui::EndChild();
                        }
                    }
                }
                ImGui::End();

                ImGui::Begin("Popupbox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
                {
                    ImGui::CustomBeginChild("antiaim!!?!", ImVec2(300, 0), ImGuiWindowFlags_NoBringToFrontOnFocus);
                    {
                        static int value = 10;
                        static float vertical = 50.f;
                        static bool checkbox = false;

                        ImGui::Checkbox("enable", &checkbox);

                        if (checkbox) {
                            ImGui::KnobInt("pitch", &value, 1, 100, "%d%%", NULL);

                            ImGui::KnobFloat("yaw", &vertical, 0.f, 100.f, "%.2fF", NULL);
                        }
                    }
                    ImGui::CustomEndChild();
                }
                ImGui::End();

                ImGui::PopStyleColor();


                ImGui::RenderNotifications();

                ImGui::EndFrame();

                return 0;
            }
        }
    }
}
