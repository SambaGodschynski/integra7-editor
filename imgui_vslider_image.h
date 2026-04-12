#pragma once
#include "imgui.h"
#include "imgui_internal.h"

namespace ImVSliderImage
{

// Vertical slider with a custom handle image.
//
// track_size   : width x height of the slider track in pixels
// handle_tex   : handle image loaded via LoadKnobTexture()
// handle_w     : display width of the handle  (may exceed track_size.x)
// handle_h     : display height of the handle
//
// Returns true if the value changed.
inline bool VSlider(
    const char* label,
    ImVec2      track_size,
    float*      p_value,
    float       v_min,
    float       v_max,
    ImTextureID handle_tex,
    float       handle_w,
    float       handle_h)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    const ImGuiID id  = window->GetID(label);
    const ImVec2  pos = window->DC.CursorPos;
    const ImRect  bb(pos, { pos.x + track_size.x, pos.y + track_size.y });

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    // -- Interaction --
    bool hovered, held;
    ImGui::ButtonBehavior(bb, id, &hovered, &held,
        ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_AllowOverlap);

    bool value_changed = false;
    if (held)
    {
        const float dy    = ImGui::GetIO().MouseDelta.y;
        const float range = v_max - v_min;
        // usable track height = total height minus handle height
        const float usable = track_size.y - handle_h;
        if (usable > 0.0f && dy != 0.0f)
        {
            *p_value -= dy * range / usable;
            *p_value  = ImClamp(*p_value, v_min, v_max);
            value_changed = true;
        }
    }

    // -- Compute handle center Y --
    const float t       = ImClamp((*p_value - v_min) / (v_max - v_min), 0.0f, 1.0f);
    const float usable  = track_size.y - handle_h;
    const float handle_cy = pos.y + handle_h * 0.5f + (1.0f - t) * usable;

    // -- Draw track --
    ImDrawList* dl       = ImGui::GetWindowDrawList();
    const float track_cx = ImFloor(pos.x + track_size.x * 0.5f);
    dl->AddRectFilled(
        { track_cx - 2.0f, pos.y },
        { track_cx + 2.0f, pos.y + track_size.y },
        IM_COL32(55, 55, 55, 255), 2.0f);

    // -- Draw handle --
    const ImVec2 h_min = { ImFloor(track_cx - handle_w * 0.5f),
                            ImFloor(handle_cy  - handle_h * 0.5f) };
    const ImVec2 h_max = { h_min.x + handle_w, h_min.y + handle_h };

    const ImU32 tint = held
        ? IM_COL32(200, 200, 255, 255)
        : (hovered ? IM_COL32(225, 225, 255, 255) : IM_COL32(255, 255, 255, 255));

    if (handle_tex != 0)
    {
        dl->AddImage(handle_tex, h_min, h_max, { 0, 0 }, { 1, 1 }, tint);
    }
    else
    {
        dl->AddRectFilled(h_min, h_max, IM_COL32(160, 160, 160, 255), 2.0f);
    }

    // -- Tooltip --
    if (hovered)
    {
        ImGui::SetTooltip("%s: %.0f", label, *p_value);
    }

    return value_changed;
}

} // namespace ImVSliderImage
