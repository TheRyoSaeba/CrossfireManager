#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../ESP/ESP.h"
#include <render.h>
 

static float ESP::RadarAngle(float delta)
{
    while (delta > M_PI)
        delta -= 2.f * M_PI;
    while (delta < -M_PI)
        delta += 2.f * M_PI;
    return delta;
}

 

void ESP::RenderRadar(Memory& mem, std::shared_ptr<ESP::Snapshot> snapshot, ImDrawList* draw)
{
    try
    {
        bool drawActive = snapshot->m_clientShell.inGame();
        if (!drawActive) return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (!snapshot) { draw_radar = FALSE; return; }

        
        static float last_radar_size_factor = radar_size_factor;
        float radarSize = viewport->Size.y * radar_size_factor;
        ImVec2 radarPos = viewport->Pos + ImVec2(20, 20);

        
        ImGui::SetNextWindowPos(radarPos, ImGuiCond_FirstUseEver);

       
        bool size_changed = last_radar_size_factor != radar_size_factor;
        ImGuiCond size_condition = size_changed ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
        ImGui::SetNextWindowSize(ImVec2(radarSize, radarSize), size_condition);

        
        last_radar_size_factor = radar_size_factor;

        ImGui::SetNextWindowBgAlpha(radar_bg_alpha);

       
        if (ImGui::Begin("Player Radar", nullptr, ImGuiWindowFlags_NoCollapse))
        
        {
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 center = windowPos + (windowSize * 0.5f);
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            ImColor radarBgColor = IM_COL32(70, 70, 70, (int)(230 * radar_opacity));
            ImColor radarOutlineColor = IM_COL32(180, 180, 180, (int)(250 * radar_opacity));
            ImColor radarGridColor = IM_COL32(80, 80, 80, (int)(180 * radar_opacity));
            ImColor radarTextColor = IM_COL32(180, 180, 180, (int)(220 * radar_opacity));
            ImColor playerColor = IM_COL32(200, 200, 200, (int)(230 * radar_opacity));
            ImColor enemyColor = IM_COL32(120, 120, 120, (int)(230 * radar_opacity));
            ImColor elevUpColor = IM_COL32(140, 140, 140, (int)(220 * radar_opacity));
            ImColor elevDownColor = IM_COL32(100, 100, 100, (int)(220 * radar_opacity));

            drawList->AddCircleFilled(center, windowSize.x * 0.45f, radarBgColor, 60);
            drawList->AddCircle(center, windowSize.x * 0.45f, radarOutlineColor, 60, 1.0f);

            for (float r = 0.15f; r <= 0.45f; r += 0.15f) {
                drawList->AddCircle(center, windowSize.x * r, radarGridColor, 60, 1.0f);
            }

            drawList->AddLine(
                ImVec2(center.x - windowSize.x * 0.45f, center.y),
                ImVec2(center.x + windowSize.x * 0.45f, center.y),
                radarGridColor, 1.0f
            );
            drawList->AddLine(
                ImVec2(center.x, center.y - windowSize.y * 0.45f),
                ImVec2(center.x, center.y + windowSize.y * 0.45f),
                radarGridColor, 1.0f
            );

            drawList->AddText(
                ImVec2(center.x - 5, center.y - windowSize.y * 0.47f),
                radarTextColor, "N"
            );
            drawList->AddText(
                ImVec2(center.x - 5, center.y + windowSize.y * 0.42f),
                radarTextColor, "S"
            );
            drawList->AddText(
                ImVec2(center.x - windowSize.x * 0.47f, center.y - 5),
                radarTextColor, "W"
            );
            drawList->AddText(
                ImVec2(center.x + windowSize.x * 0.42f, center.y - 5),
                radarTextColor, "E"
            );

            if (!draw_enemies_as_arrows) {

                drawList->AddCircleFilled(center, 5.0f, IM_COL32(255, 255, 255, 255));
            }
            else {

                drawList->AddTriangleFilled(
                    ImVec2(center.x, center.y - 10.0f),
                    ImVec2(center.x - 7.0f, center.y + 7.0f),
                    ImVec2(center.x + 7.0f, center.y + 7.0f),
                    IM_COL32(255, 255, 255, 255)
                );
            }

            float baseVisibleRange = 100.0f * 100.0f;
            float visibleRange = baseVisibleRange / radar_zoom;
            float scale = (windowSize.x * 0.45f) / visibleRange;

            for (auto& enemy : snapshot->enemies) {
                if (enemy.IsDead || enemy.Team == snapshot->localTeam) continue;

                float dx = enemy.AbsPos.x - snapshot->localAbsPos.x;
                float dz = enemy.AbsPos.z - snapshot->localAbsPos.z;

                float tempX = dz;
                float tempZ = -dx;
                dx = tempX;
                dz = tempZ;

                float angleToEnemy = atan2f(-dz, dx);
                float relativeAngle = angleToEnemy - snapshot->localYaw;

                float distSq = dx * dx + dz * dz;
                float dist = sqrtf(distSq);

                float distMeters = dist / 100.0f;

                if (distMeters > radarMaxDistance)
                    continue;

                float radarX = sinf(relativeAngle) * dist * scale;
                float radarY = -cosf(relativeAngle) * dist * scale;

                ImVec2 blipPos = center + ImVec2(radarX, radarY);

                if (draw_enemies_as_arrows) {

                    drawList->AddCircleFilled(blipPos, 2.0f, IM_COL32(255, 40, 40, 255));

                    ImVec2 toCenter = ImVec2(center.x - blipPos.x, center.y - blipPos.y);
                    float len = sqrtf(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
                    if (len > 0.0001f) {
                        toCenter.x /= len;
                        toCenter.y /= len;
                    }
                    else {
                        toCenter.x = 0;
                        toCenter.y = 1;
                    }

                    float arrowLength = 15.0f;
                    float arrowWidth = 6.0f;

                    ImVec2 arrowTip = blipPos + toCenter * arrowLength;

                    ImVec2 perp(-toCenter.y, toCenter.x);

                    ImVec2 baseLeft = blipPos + perp * arrowWidth;
                    ImVec2 baseRight = blipPos - perp * arrowWidth;

                    drawList->AddTriangleFilled(
                        arrowTip,
                        baseLeft,
                        baseRight,
                        IM_COL32(255, 40, 40, 255)
                    );
                }
                else {
                    drawList->AddCircleFilled(blipPos, 5.0f, IM_COL32(255, 40, 40, 255));
                }

                float elevDiff = (enemy.AbsPos.y - snapshot->localAbsPos.y) / 100.0f;
                

 
                if (fabsf(elevDiff) > radar_elev_threshold) {
                    const char* elevText = elevDiff > 0 ? "UP" : "DN";
                    ImColor elevColor = elevDiff > 0 ?
                        ImColor(100, 255, 100, 255) : ImColor(100, 150, 255, 255);

                    ImVec2 textSize = ImGui::CalcTextSize(elevText);

                    float vertOffset = draw_enemies_as_arrows ? 8.0f + 2.0f : 6.0f + 2.0f;

                    float yPos = elevDiff > 0 ?
                        blipPos.y - vertOffset - textSize.y :
                        blipPos.y + vertOffset;

                    drawList->AddText(
                        ImVec2(blipPos.x - textSize.x / 2, yPos),
                        elevColor,
                        elevText
                    );
                }

                
                if (draw_enemy_names && enemy.Name[0] != '\0') {
                    ImVec2 textSize = ImGui::CalcTextSize(enemy.Name);

                    drawList->AddText(
                        ImVec2(blipPos.x + 5.0f + 2, blipPos.y - textSize.y / 2),
                        IM_COL32(255, 255, 255, 255),
                        enemy.Name
                    );
                }
            }

             

            ImGui::End();
            
        }
    }
    catch (...) {}
}