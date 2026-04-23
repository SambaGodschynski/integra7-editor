#pragma once
#include "imgui.h"
#include "imgui_internal.h"

namespace ImDrawbar
{

// Drawbar widget simulating a Hammond organ drawbar.
//
// The image (42 x 256 px native) shows the drawbar shaft (numbers 8→1 top→bottom)
// with a handle at the very bottom. The slot clips the image and the image slides
// so that:
//   value = v_min  →  only the handle is visible at the top of the slot
//   value = v_max  →  shaft + handle visible, handle near bottom of slot
//
// Drag DOWN to increase value (pull out = louder).
//
// img_w          : display width (slot width)
// slot_h         : visible height of the slot window
// img_native_h   : native image height in pixels (256)
// img_handle_h   : height of the handle element in native pixels (~45)
//
// Returns true if value changed.
// snap_offset: visual pixel offset added to the handle position (display pixels).
// Use to fine-tune alignment when the handle image does not land exactly on integer steps.
inline bool Drawbar(
    const char* label,
    float*      p_value,
    float       v_min,
    float       v_max,
    ImTextureID image_tex,
    float       img_w,
    float       slot_h,
    float       img_native_h,
    float       img_handle_native_h,
    float       snap_offset = 0.0f)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) { return false; }

    const ImGuiID id  = window->GetID(label);
    const ImVec2  pos = window->DC.CursorPos;
    const ImRect  bb(pos, { pos.x + img_w, pos.y + slot_h });

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) { return false; }

    bool hovered, held;
    ImGui::ButtonBehavior(bb, id, &hovered, &held,
        ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_AllowOverlap);

    // Scale factor: native → display
    const float scale      = img_w / 42.0f;
    const float img_disp_h = img_native_h * scale;
    const float hdl_disp_h = img_handle_native_h * scale;
    // Travel: handle stays within the slot at all values.
    // kSlotH should be >= img_disp_h so that at t=1 the full shaft is reachable.
    const float travel = slot_h - hdl_disp_h;

    bool value_changed = false;
    if (held && travel > 0.0f)
    {
        const float dy = ImGui::GetIO().MouseDelta.y;
        if (dy != 0.0f)
        {
            const float prev_step = roundf(*p_value);
            *p_value += dy * (v_max - v_min) / travel;
            *p_value  = ImClamp(*p_value, v_min, v_max);
            const float new_step = roundf(*p_value);
            if (new_step != prev_step)
            {
                // Snap the stored value so the next drag starts from a clean integer.
                *p_value     = new_step;
                value_changed = true;
            }
        }
    }

    // Snap to nearest integer for display so receive-induced float values don't drift visually.
    const float snapped = ImClamp(roundf(*p_value), v_min, v_max);
    const float t = (v_max > v_min)
        ? ImClamp((snapped - v_min) / (v_max - v_min), 0.0f, 1.0f)
        : 0.0f;

    // Image top position:
    //   t=0 → image is pulled UP so only handle (bottom of image) appears at slot top
    //   t=1 → image slides down so handle is near slot bottom, shaft visible
    const float img_top = pos.y - (img_disp_h - hdl_disp_h) + t * travel + snap_offset;
    const float img_bot = img_top + img_disp_h;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Slot background
    dl->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + img_w, pos.y + slot_h },
        IM_COL32(22, 22, 22, 255), 3.0f);

    // Draw image clipped to slot
    dl->PushClipRect({ pos.x, pos.y }, { pos.x + img_w, pos.y + slot_h }, true);

    const ImU32 tint = held
        ? IM_COL32(200, 200, 255, 255)
        : (hovered ? IM_COL32(230, 230, 255, 255) : IM_COL32(255, 255, 255, 255));

    if (image_tex != 0)
    {
        dl->AddImage(image_tex,
            { pos.x, img_top }, { pos.x + img_w, img_bot },
            { 0, 0 }, { 1, 1 }, tint);
    }
    else
    {
        const float handle_y = img_top + (img_disp_h - hdl_disp_h);
        dl->AddRectFilled(
            { pos.x + 4.0f, handle_y },
            { pos.x + img_w - 4.0f, handle_y + hdl_disp_h },
            IM_COL32(160, 160, 160, 255), 3.0f);
    }

    dl->PopClipRect();

    return value_changed;
}

} // namespace ImDrawbar
