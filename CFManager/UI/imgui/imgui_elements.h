#define IMGUI_DEFINE_MATH_OPERATORS

#include <cmath>
#include <cstdlib>
#include <imgui.h>
#include <imgui_internal.h>
#include "imgui_settings.h"
#include <imgui_knobs.h>
#include <map>

struct slider_state {
    ImVec2 grab_bb_center_offset;
    ImVec4 text_color;
};

namespace ImGui
{

    struct slider_state {
        ImVec4 background, circle, text;
        float slow_anim, circle_anim;
        float position;
    };

    inline bool KnobScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = GetContentRegionMax().x - style.WindowPadding.x;

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos + ImVec2(0, 0), window->DC.CursorPos + ImVec2(w, label_size.y + 30.f));

        const ImRect slider_bb(window->DC.CursorPos + ImVec2(w - 30, 0), window->DC.CursorPos + ImVec2(w, 100));

        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? label_size.x : 0.0f, 0.0f));

        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
        ItemSize(ImRect(total_bb.Min, total_bb.Max - ImVec2(0, 25)));

        if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
            return false;

        if (format == NULL) format = DataTypeGetInfo(data_type)->PrintFmt;

        const bool hovered = ItemHoverable(frame_bb, id, NULL);
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
        if (!temp_input_is_active)
        {
            const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
            const bool clicked = hovered && IsMouseClicked(0, id);
            const bool make_active = (input_requested_by_tabbing || clicked || g.NavActivateId == id);
            if (make_active && clicked)
                SetKeyOwner(ImGuiKey_MouseLeft, id);
            if (make_active && temp_input_allowed)
                if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
                    temp_input_is_active = true;

            if (make_active && !temp_input_is_active)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            }
        }

        ImRect grab_bb;

        static std::map<ImGuiID, slider_state> anim;
        auto it_anim = anim.find(id);

        if (it_anim == anim.end())
        {
            anim.insert({ id, slider_state() });
            it_anim = anim.find(id);
        }

        it_anim->second.circle_anim = ImLerp(it_anim->second.circle_anim, IsItemActive() ? 11.f : 10.f, g.IO.DeltaTime * 6.f);
        it_anim->second.text = ImLerp(it_anim->second.text, IsItemActive() ? c::text::text_active : hovered ? c::text::text_hov : c::text::text, g.IO.DeltaTime * 6.f);


        const bool value_changed = DragBehavior(id, data_type, p_data, 0.f, p_min, p_max, format, NULL);
        if (value_changed) MarkItemEdited(id);

        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

        static float arc_size = 0.45f; 
        static float radius = 10.f;
        static float thickness = 4.f;

        it_anim->second.position = ImLerp(it_anim->second.position, *static_cast<float*>(p_data) / *reinterpret_cast<const float*>(p_max) * 6.35f, ImGui::GetIO().DeltaTime * 18.f);

        window->DrawList->PathClear();
        window->DrawList->PathArcTo(ImVec2(frame_bb.Max.x + radius - 23, frame_bb.Min.y + (30 / 2)), radius, 0.f, 2.f * IM_PI, 40.f);
        window->DrawList->PathStroke(GetColorU32(c::slider::background), 0, thickness);

        window->DrawList->PathClear();
        window->DrawList->PathArcTo(ImVec2(frame_bb.Max.x + radius - 23, frame_bb.Min.y + (30 / 2)), radius, IM_PI * 1.47f, IM_PI * 1.5f + it_anim->second.position, 40.f);
        window->DrawList->PathStroke(GetColorU32(c::accent), 0, thickness);

        window->DrawList->AddCircleFilled(ImVec2(frame_bb.Max.x + radius - 23 + ImCos(IM_PI * 1.5f + it_anim->second.position) * radius, frame_bb.Min.y + (30 / 2) + ImSin(IM_PI * 1.5f + it_anim->second.position) * radius), 3.f, GetColorU32(c::accent));
        window->DrawList->AddCircleFilled(ImVec2(frame_bb.Max.x + radius - 23 + ImCos(IM_PI * 1.5f + it_anim->second.position) * radius, frame_bb.Min.y + (30 / 2) + ImSin(IM_PI * 1.5f + it_anim->second.position) * radius), 2.f, GetColorU32(c::slider::background));

        GetWindowDrawList()->AddText(ImVec2(frame_bb.Max.x - (35 + CalcTextSize(value_buf).x), frame_bb.Min.y + (15 / 2) + 2), GetColorU32(c::text::text_active), value_buf);

        GetWindowDrawList()->AddText(ImVec2(frame_bb.Max.x - w, frame_bb.Min.y + (15 / 2) + 2), GetColorU32(it_anim->second.text), label);

        return value_changed;
    }


    inline bool KnobFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return KnobScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }

    inline  bool KnobInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return KnobScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
    }

}