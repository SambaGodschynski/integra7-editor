#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-knobs.h"
#include <cstdio>

// Implemented in imgui_knob_image.cpp (needs stb_image + OpenGL).
ImTextureID LoadKnobTexture(const char* path);

namespace ImKnobImage
{

// Draws an atlas-based knob with the same interaction model as ImGuiKnobs::Knob.
//
// atlas        : grid sprite atlas loaded via LoadKnobTexture()
// atlas_frames : total number of frames (frame 0 = min, frame N-1 = max)
// atlas_cols   : frames per row in the grid
// size         : knob diameter in pixels (0 = auto from font size)
//
// Returns true if the value changed.
inline bool Knob(
    const char*    label,
    float*         p_value,
    float          v_min,
    float          v_max,
    float          speed,
    const char*    format,
    ImTextureID    atlas,
    int            atlas_frames,
    int            atlas_cols,
    float          size  = 0.0f,
    ImGuiKnobFlags flags = 0)
{
#if IMGUI_VERSION_NUM < 19197
    const float font_scale = ImGui::GetIO().FontGlobalScale;
#else
    const float font_scale = ImGui::GetStyle().FontScaleMain;
#endif
    const float diameter  = (size == 0.0f)
        ? ImGui::GetTextLineHeight() * 4.0f
        : size * font_scale;
    const float radius    = diameter * 0.5f;
    const float act_speed = (speed == 0.0f) ? (v_max - v_min) / 250.0f : speed;

    ImGui::PushID(label);
    ImGui::BeginGroup();
    ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = 0;

    // -- Title --
    if (!(flags & ImGuiKnobFlags_NoTitle))
    {
        const ImVec2 title_size = ImGui::CalcTextSize(label, NULL, false, diameter);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (diameter - title_size.x) * 0.5f);
        ImGui::TextUnformatted(label);
    }

    // -- Interactive area --
    const ImVec2 screen_pos = ImGui::GetCursorScreenPos();
    const ImVec2 p_min = { ImFloor(screen_pos.x), ImFloor(screen_pos.y) };
    const ImVec2 p_max = { p_min.x + diameter,    p_min.y + diameter    };

    const ImGuiID gid = ImGui::GetID("##k");
    ImGui::InvisibleButton("##k", { diameter, diameter });

    const bool is_active  = ImGui::IsItemActive();
    const bool is_hovered = ImGui::IsItemHovered();

    // -- Drag behavior --
    const ImGuiIO& io = ImGui::GetIO();
    const bool drag_vertical =
        !(flags & ImGuiKnobFlags_DragHorizontal) &&
        ((flags & ImGuiKnobFlags_DragVertical) ||
         ImAbs(io.MouseDelta.y) > ImAbs(io.MouseDelta.x));

    ImGuiSliderFlags drag_flags = 0;
    if (drag_vertical)                      { drag_flags |= ImGuiSliderFlags_Vertical;    }
    if (flags & ImGuiKnobFlags_AlwaysClamp) { drag_flags |= ImGuiSliderFlags_AlwaysClamp; }

    const bool value_changed = ImGui::DragBehavior(
        gid, ImGuiDataType_Float, p_value,
        act_speed, &v_min, &v_max, format, drag_flags);

    // -- Draw atlas frame --
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (atlas != 0 && atlas_frames > 0 && atlas_cols > 0)
    {
        float t = (*p_value - v_min) / (v_max - v_min);
        t = ImClamp(t, 0.0f, 1.0f);
        const int frame = ImClamp((int)(t * (atlas_frames - 1) + 0.5f), 0, atlas_frames - 1);

        // Grid layout: atlas_cols frames per row.
        const int atlas_rows = (atlas_frames + atlas_cols - 1) / atlas_cols;
        const int col = frame % atlas_cols;
        const int row = frame / atlas_cols;
        const float u0 = (float)col       / atlas_cols;
        const float u1 = (float)(col + 1) / atlas_cols;
        const float v0 = (float)row       / atlas_rows;
        const float v1 = (float)(row + 1) / atlas_rows;

        const ImU32 tint = is_active
            ? IM_COL32(200, 200, 255, 255)
            : (is_hovered ? IM_COL32(225, 225, 255, 255) : IM_COL32(255, 255, 255, 255));
        dl->AddImage(atlas, p_min, p_max, { u0, v0 }, { u1, v1 }, tint);
    }
    else
    {
        // Fallback: plain circle so the layout stays intact without an atlas.
        const ImVec2 center = { p_min.x + radius, p_min.y + radius };
        dl->AddCircleFilled(center, radius,
            is_active ? IM_COL32(120, 120, 200, 255) : IM_COL32(80, 80, 80, 255));
    }

    // -- Value drag-input below (unless NoInput) --
    if (!(flags & ImGuiKnobFlags_NoInput))
    {
        ImGui::SetNextItemWidth(diameter);
        ImGui::DragScalar("##v", ImGuiDataType_Float, p_value,
            act_speed, &v_min, &v_max, format, drag_flags);
    }

    // -- Tooltip --
    if (is_hovered)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), format, *p_value);
        ImGui::SetTooltip("%s: %s", label, buf);
    }

    ImGui::EndGroup();
    ImGui::PopID();

    return value_changed;
}

} // namespace ImKnobImage
