#pragma once

#include "imgui.h"
#include "imgui_internal.h"

// Two-handle horizontal range slider.
// Each handle is an InvisibleButton -- ImGui keeps it active while the mouse
// button is held, regardless of where the mouse moves.
// Returns true if either value changed.
// label: used for ID scoping (use ##prefix to suppress text display)
// v_lo / v_hi: current values, modified in place
// v_min / v_max: hard bounds
inline bool RangeSliderInt(
    const char* label,
    int*        v_lo,
    int*        v_hi,
    int         v_min,
    int         v_max)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;

    const float availW = ImGui::CalcItemWidth();
    const float frameH = ImGui::GetFrameHeight();
    const ImVec2 pos   = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + frameH));

    // Reserve layout space upfront so subsequent widgets are placed correctly.
    ImGui::ItemSize(bb, style.FramePadding.y);

    const float handleR  = frameH * 0.38f;
    const float trackH   = 5.0f;
    const float trackX0  = bb.Min.x + handleR;
    const float trackX1  = bb.Max.x - handleR;
    const float midY     = bb.Min.y + frameH * 0.5f;
    const float trackY0  = midY - trackH * 0.5f;
    const float trackY1  = midY + trackH * 0.5f;
    const float trackSpan = trackX1 - trackX0;
    const float valSpan   = (float)(v_max - v_min);

    auto valToX = [&](int v) -> float
    {
        return trackX0 + (float)(v - v_min) / valSpan * trackSpan;
    };

    auto xToVal = [&](float x) -> int
    {
        float t = (x - trackX0) / trackSpan;
        if (t < 0.0f) { t = 0.0f; }
        if (t > 1.0f) { t = 1.0f; }
        return v_min + (int)(t * valSpan + 0.5f);
    };

    bool changed = false;

    ImGui::PushID(label);

    // ---- Lo handle ----
    // InvisibleButton stays active (IsItemActive==true) while mouse is held,
    // even when the mouse moves outside the button's original bounding box.
    float loX = valToX(*v_lo);
    ImGui::SetCursorScreenPos(ImVec2(loX - handleR, midY - handleR));
    ImGui::InvisibleButton("lo", ImVec2(handleR * 2.0f, handleR * 2.0f));
    const bool loActive  = ImGui::IsItemActive();
    const bool loHovered = ImGui::IsItemHovered();

    if (loActive && ImGui::IsMousePosValid())
    {
        int nv = xToVal(g.IO.MousePos.x);
        if (nv < v_min) { nv = v_min; }
        if (nv > *v_hi) { nv = *v_hi; }
        if (nv != *v_lo) { *v_lo = nv; changed = true; }
        loX = valToX(*v_lo);
    }

    // ---- Hi handle ----
    float hiX = valToX(*v_hi);
    ImGui::SetCursorScreenPos(ImVec2(hiX - handleR, midY - handleR));
    ImGui::InvisibleButton("hi", ImVec2(handleR * 2.0f, handleR * 2.0f));
    const bool hiActive  = ImGui::IsItemActive();
    const bool hiHovered = ImGui::IsItemHovered();

    if (hiActive && ImGui::IsMousePosValid())
    {
        int nv = xToVal(g.IO.MousePos.x);
        if (nv > v_max) { nv = v_max; }
        if (nv < *v_lo) { nv = *v_lo; }
        if (nv != *v_hi) { *v_hi = nv; changed = true; }
        hiX = valToX(*v_hi);
    }

    ImGui::PopID();

    // Restore cursor to below the reserved space.
    ImGui::SetCursorScreenPos(ImVec2(bb.Min.x, bb.Max.y));

    // ---- Draw (after interaction so positions are current) ----
    ImDrawList* dl = window->DrawList;

    // Track background
    dl->AddRectFilled(
        ImVec2(trackX0, trackY0), ImVec2(trackX1, trackY1),
        ImGui::GetColorU32(ImGuiCol_FrameBg), trackH * 0.5f);

    // Active range fill
    dl->AddRectFilled(
        ImVec2(loX, trackY0), ImVec2(hiX, trackY1),
        ImGui::GetColorU32(ImGuiCol_SliderGrab), trackH * 0.5f);

    // Lo handle circle
    dl->AddCircleFilled(ImVec2(loX, midY), handleR,
        ImGui::GetColorU32((loActive || loHovered)
            ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab));

    // Hi handle circle
    dl->AddCircleFilled(ImVec2(hiX, midY), handleR,
        ImGui::GetColorU32((hiActive || hiHovered)
            ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab));

    // Tooltip
    if (loHovered || hiHovered || loActive || hiActive)
    {
        ImGui::SetTooltip("Lo: %d  Hi: %d", *v_lo, *v_hi);
    }

    return changed;
}
