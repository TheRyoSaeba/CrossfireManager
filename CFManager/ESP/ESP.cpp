#include "ESP.h"
using namespace std::chrono_literals;
void ESP::Render(Memory& mem, std::shared_ptr<ESP::Snapshot> snapshot, ImDrawList* draw)
{
    bool drawActive = snapshot->m_clientShell.inGame();

    if (!drawActive) {
        draw->CmdBuffer.clear();
        draw->VtxBuffer.clear();
        draw->IdxBuffer.clear();
    }
    else {


        if (snapshot) {
            for (const auto& enemy : snapshot->enemies) {

                if (!snapshot->m_clientShell.inGame())
                {
                    continue;

                }


                bool isAlly = (enemy.Team == snapshot->localTeam);

                if (Filterteams && isAlly)
                    continue;

                if (enemy.IsDead)
                    continue;


                D3DXVECTOR3 headPos = enemy.HeadPos;
                D3DXVECTOR3 footPos = enemy.FootPos;



                if (!EngineW2S(snapshot->drawPrim, &headPos) || !EngineW2S(snapshot->drawPrim, &footPos))
                    continue;


                float height = fabs(footPos.y - headPos.y) * 1.15f;
                float width = height * 0.6f;

                float x = headPos.x - width / 2;
                float y = headPos.y - (height * 0.15f);

                RGBA finalColor = isAlly ? g_AllyColor : g_EnemyColor;


                float dx = enemy.AbsPos.x - snapshot->localAbsPos.x;
                float dy = enemy.AbsPos.y - snapshot->localAbsPos.y;
                float dz = enemy.AbsPos.z - snapshot->localAbsPos.z;
                float distanceMeters = std::fmaxf((sqrtf(dx * dx + dy * dy + dz * dz) - 400.0f) / 100.0f, 0.0f);

                float scaleFactor = std::clamp(1.5f - (distanceMeters / 50.0f), 0.6f, 1.2f);

                if (fov == 0 || distanceMeters > fov) continue;


                std::string playerName = "Player"; 

                if (enemy.Name[0] != '\0' && std::isprint(enemy.Name[0])) {
                    playerName = std::string(enemy.Name, strnlen(enemy.Name, sizeof(enemy.Name)));

                    
                    playerName.erase(std::remove_if(playerName.begin(), playerName.end(), [](char c) {
                        return !(std::isprint(static_cast<unsigned char>(c)));
                        }), playerName.end());

                    if (playerName.empty())
                        playerName = "Player";
                }


                RectData rect;
                rect.x = x;
                rect.y = y;
                rect.w = width;
                rect.h = height;
                rect.color = finalColor;
                rect.playerName = playerName;
                rect.currentHP = enemy.Health;
                rect.maxHP = 100;
                rect.headPos = headPos;
                rect.isAlly = (enemy.Team == snapshot->localTeam);

                if (Flogs[0]) DrawHeadCircle(rect, draw, snapshot->drawPrim);
                if (Dcheckbox) DrawBoxESP(rect, draw, scaleFactor,esptype);
                if (showEspLines && !rect.isAlly) DrawTraceline(rect, *snapshot, draw);
                if (Flogs[1]) DrawHealthBar(rect, static_cast<float>(enemy.Health), draw);
                if (Flogs[2]) DrawNameESP(rect, draw);
                if (Flogs[3])DrawDistance(rect, distanceMeters, draw);
                if (Bonecheckbox)
                    DrawBones(mem, *snapshot, draw);
                
            }
        }

    }

}

void ESP::DrawHeadCircle(const RectData& rect, ImDrawList* draw, const LT_DRAWPRIM& drawPrim)
{
    ImVec2 crosshairCenter(
        drawPrim.viewport.X + drawPrim.viewport.Width * 0.5f,
        drawPrim.viewport.Y + drawPrim.viewport.Height * 0.5f
    );

    float dx = rect.headPos.x - crosshairCenter.x;
    float dy = rect.headPos.y - crosshairCenter.y;
    float distance = sqrtf(dx * dx + dy * dy);

    float headRadius = 0.4f + hdtk;
    float threshold = 40.0f;

    ImVec2 headCenter(rect.headPos.x - 3.0f, rect.headPos.y - 8.0f);

    draw->AddCircleFilled(
        headCenter,
        headRadius,
        IM_COL32(g_HeadColor.R, g_HeadColor.G, g_HeadColor.B, g_HeadColor.A)
    );

    if (distance < threshold && crosshair_notify && !rect.isAlly)
    {
        ImVec2 shootPos(rect.headPos.x + 20.0f, rect.headPos.y - 25.0f);

       //Gui::PushFont(font::tahoma_bold2);
        draw->AddText(shootPos, IM_COL32(255, 0, 0, 255), "Shoot!");
       // ImGui::PopFont();
    }
}
 void ESP::DrawCornerBox(int x, int y, int w, int h, float borderPx, RGBA color) {
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

void ESP::DrawBoxESP(const RectData& rect, ImDrawList* draw, float scaleFactor, int esptype)
{
    switch (esptype) {
    case 0:
        draw->AddRect(
            ImVec2(rect.x, rect.y),
            ImVec2(rect.x + rect.w, rect.y + rect.h),
            IM_COL32(rect.color.R, rect.color.G, rect.color.B, rect.color.A),
            0.0f, ImDrawFlags_Closed, boxtk * scaleFactor
        );
        break;

    case 1:
        DrawCornerBox(rect.x, rect.y, rect.w, rect.h, boxtk * scaleFactor, rect.color);
        break;

    case 2:
        draw->AddRectFilled(
            ImVec2(static_cast<float>(rect.x), static_cast<float>(rect.y)),
            ImVec2(static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h)),
            IM_COL32(rect.color.R, rect.color.G, rect.color.B, rect.color.A / 3)
        );

        draw->AddRect(
            ImVec2(static_cast<float>(rect.x), static_cast<float>(rect.y)),
            ImVec2(static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h)),
            IM_COL32(rect.color.R, rect.color.G, rect.color.B, rect.color.A),
            0.0f, ImDrawFlags_Closed, boxtk * scaleFactor
        );
        break;
    }
}

void ESP::DrawTraceline(const RectData& rect, const ESP::Snapshot& snapshot, ImDrawList* draw)
{
    
    if (rect.isAlly)
        return;

    const auto& PRIME = snapshot.drawPrim;
    int vpWidth = PRIME.viewport.Width;
    int vpHeight = PRIME.viewport.Height;

    ImVec2 viewportCenter(vpWidth * 0.5f, vpHeight);
    ImVec2 enemyScreenPos(rect.x + rect.w / 2, rect.y + rect.h / 2);

    float midX = (viewportCenter.x + enemyScreenPos.x) / 2.0f;

    float distance = sqrtf(
        powf(enemyScreenPos.x - viewportCenter.x, 2) +
        powf(enemyScreenPos.y - viewportCenter.y, 2));

    float arcHeight = distance * 0.1f;
    float animOffset = sinf(ImGui::GetTime() * 3.0f) * 10.0f;
    float midY = std::min(viewportCenter.y, enemyScreenPos.y) - arcHeight + animOffset;

    ImVec2 p2(midX, midY);
    ImVec2 p3(midX, midY);

    ImU32 colU32 = IM_COL32(
        g_ESPLineColor.R,
        g_ESPLineColor.G,
        g_ESPLineColor.B,
        g_ESPLineColor.A);

    draw->AddBezierCubic(viewportCenter, p2, p3, enemyScreenPos, colU32, 6.0f, 10);
}

static ImU32 GetHealthColor(float healthPercent)
{
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

void ESP::DrawHealthBar(const RectData& rect, float health, ImDrawList* draw)
{
    float healthPercent = std::clamp(health / 100.0f, 0.0f, 1.0f);

    float barWidth = 4.0f + hptk;
    float barHeight = rect.h * healthPercent;
    float barX = rect.x - barWidth - 5;
    float barY = rect.y + rect.h - barHeight;

     
    draw->AddRectFilled(
        ImVec2(barX, rect.y),
        ImVec2(barX + barWidth, rect.y + rect.h),
        IM_COL32(50, 50, 50, 150)
    );

    
    draw->AddRectFilled(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, rect.y + rect.h),
        GetHealthColor(healthPercent)
    );

     
    draw->AddRect(
        ImVec2(barX, rect.y),
        ImVec2(barX + barWidth, rect.y + rect.h),
        IM_COL32(255, 255, 255, 255),
        0.0f, 0, 1.0f
    );
}

void ESP::DrawNameESP(const RectData& rect, ImDrawList* draw)
{
    if (rect.playerName.empty())
        return;

    if (Flogs[2]) {
        ImVec2 textSize = ImGui::CalcTextSize(rect.playerName.c_str());
        draw->AddText(ImVec2(rect.x + (rect.w - textSize.x) * 0.5f, rect.y - textSize.y - 2.0f),
            IM_COL32(g_NameColor.R, g_NameColor.G, g_NameColor.B, g_NameColor.A)
            , rect.playerName.c_str());
    }
}

void ESP::DrawDistance(const RectData& rect, float distance, ImDrawList* draw)
{
    char buffer[32];
    sprintf(buffer, "%.1fm", distance);

    ImVec2 textSize = ImGui::CalcTextSize(buffer);
    float textX = rect.x + (rect.w - textSize.x) * 0.5f;
    float textY = rect.y + rect.h + 2.0f;

    draw->AddText(ImVec2(textX, textY),
        IM_COL32(255, 255, 255, 255),
        buffer);
}
static void DrawAllBones( ImDrawList* draw,Memory& mem,uintptr_t hObject, const std::vector<int>& boneIDs,const std::array<D3DXVECTOR3, NUM_BONES>& positions,const KLASSES::LT_DRAWPRIM& drawPrim,
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

        if (!KLASSES::EngineW2S(drawPrim, &posA)) continue;
        if (!KLASSES::EngineW2S(drawPrim, &posB)) continue;

        float bonethick = 1.5f + bonetk;

        draw->AddLine(
            ImVec2(posA.x, posA.y),
            ImVec2(posB.x, posB.y),
            color,
            bonethick
        );
    }
}
void ESP::DrawBones(Memory& mem, const ESP::Snapshot& snapshot, ImDrawList* draw)
{
    std::vector<int> boneGroups = {
         6,  5,  4,  3,  1,
        21, 22, 23, 25, 26,
        27, 14, 15, 16, 17,
         7, 8, 9, 10
    };

    for (const auto& enemy : snapshot.enemies)
    {
        if (enemy.IsDead) continue;
        if (Filterteams && enemy.Team == snapshot.localTeam) continue;

        RGBA color = (enemy.Team == snapshot.localTeam) ? g_AllyColor : g_HeadColor;

        DrawAllBones(
            draw,
            mem,
            reinterpret_cast<uintptr_t>(enemy.hObject),
            boneGroups,
            enemy.bones,
            snapshot.drawPrim,
            IM_COL32(color.R, color.G, color.B, color.A)
        );
    }
}
