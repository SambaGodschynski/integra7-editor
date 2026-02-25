#pragma once
#include "imgui.h"
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

namespace ImStepLfo
{

struct DragState
{
    int   stepIdx  = -1;
    float startVal = 0.f;
    float startY   = 0.f;
};

// Draws a step-LFO sequencer bar chart.
// steps    : array of nSteps float* pointers  (LFO_STEP1 .. LFO_STEP16)
// stepType : 0 = TYPE1 (square/hold),  1 = TYPE2 (ramp/interpolated)
// valMin/valMax : GUI range (e.g. -36 / +36)
// Returns true when any step value was changed by the user.
inline bool StepLfoWidget(
    const char* strId,
    float**     steps,
    int         nSteps,
    float       stepType,
    float       valMin,
    float       valMax,
    ImVec2      size = ImVec2(0.f, 80.f))
{
    bool changed = false;
    ImGui::PushID(strId);

    if (size.x <= 0.f)
    {
        size.x = ImGui::GetContentRegionAvail().x;
    }

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("canvas", size);
    const bool    isActive = ImGui::IsItemActive();
    const ImGuiID wid      = ImGui::GetItemID();

    ImDrawList* dl      = ImGui::GetWindowDrawList();
    ImVec2      rectMax = ImVec2(origin.x + size.x, origin.y + size.y);
    const float range   = valMax - valMin;
    const float barW    = size.x / (float)nSteps;
    const float zeroY   = origin.y + (1.f - (-valMin) / range) * size.y;

    // Background
    dl->AddRectFilled(origin, rectMax, IM_COL32(25, 25, 25, 255));
    dl->AddRect      (origin, rectMax, IM_COL32(80, 80, 80, 255));

    // Faint grid lines at +/-half, +/-max
    auto gridLine = [&](float val)
    {
        float y = origin.y + (1.f - (val - valMin) / range) * size.y;
        dl->AddLine(ImVec2(origin.x, y), ImVec2(rectMax.x, y),
                    IM_COL32(50, 50, 50, 255));
    };
    gridLine(valMax * 0.5f);
    gridLine(valMin * 0.5f);

    // Zero line (prominent)
    dl->AddLine(ImVec2(origin.x, zeroY), ImVec2(rectMax.x, zeroY),
                IM_COL32(70, 70, 70, 255));

    // Drag state
    static std::map<ImGuiID, DragState> sDragStates;
    DragState& drag = sDragStates[wid];

    const ImVec2 mousePos    = ImGui::GetMousePos();
    const bool   mouseInWidget =
        mousePos.x >= origin.x && mousePos.x <= rectMax.x &&
        mousePos.y >= origin.y && mousePos.y <= rectMax.y;

    // Which bar is under the mouse?
    int barUnderMouse = -1;
    if (mouseInWidget)
    {
        int b = (int)((mousePos.x - origin.x) / barW);
        barUnderMouse = std::clamp(b, 0, nSteps - 1);
    }

    // On first active frame: latch the bar
    if (ImGui::IsItemActivated())
    {
        drag.stepIdx  = barUnderMouse;
        drag.startVal = (drag.stepIdx >= 0) ? *steps[drag.stepIdx] : 0.f;
        drag.startY   = mousePos.y;
    }

    // While dragging: update value
    if (isActive && drag.stepIdx >= 0)
    {
        float dy = mousePos.y - drag.startY;
        float newVal = std::round(std::clamp(
            drag.startVal - (dy / size.y) * range,
            valMin, valMax));
        if (newVal != *steps[drag.stepIdx])
        {
            *steps[drag.stepIdx] = newVal;
            changed = true;
        }
    }

    // Draw bars
    for (int i = 0; i < nSteps; ++i)
    {
        const float val   = *steps[i];
        const float barX  = origin.x + i * barW;
        const float valY  = origin.y + (1.f - (val - valMin) / range) * size.y;
        const bool  isDragging = isActive && drag.stepIdx == i;
        const bool  isHovered  = !isActive && barUnderMouse == i;

        // Fill colour: positive = blue, negative = orange; brighter when active
        ImU32 fillCol;
        if (isDragging)
        {
            fillCol = IM_COL32(255, 200, 0, 200);
        }
        else if (isHovered)
        {
            fillCol = (val >= 0.f) ? IM_COL32(80, 160, 255, 200)
                                   : IM_COL32(255, 140, 60, 200);
        }
        else
        {
            fillCol = (val >= 0.f) ? IM_COL32(50, 120, 220, 180)
                                   : IM_COL32(220, 100, 40, 180);
        }

        float barLeft  = barX + 1.f;
        float barRight = barX + barW - 1.f;
        float top      = std::min(valY, zeroY);
        float bot      = std::max(valY, zeroY);
        if (bot - top < 1.f)
        {
            bot = top + 1.f;  // always at least 1px visible
        }
        dl->AddRectFilled(ImVec2(barLeft, top), ImVec2(barRight, bot), fillCol);
    }

    // Waveform overlay polyline
    {
        std::vector<ImVec2> poly;
        if ((int)stepType == 0)
        {
            // TYPE1: staircase (square/hold)
            poly.reserve(nSteps * 2);
            for (int i = 0; i < nSteps; ++i)
            {
                float y  = origin.y + (1.f - (*steps[i] - valMin) / range) * size.y;
                float x0 = origin.x + i * barW;
                float x1 = origin.x + (i + 1) * barW;
                poly.push_back(ImVec2(x0, y));
                poly.push_back(ImVec2(x1, y));
            }
        }
        else
        {
            // TYPE2: linear ramp between bar centres
            poly.reserve(nSteps);
            for (int i = 0; i < nSteps; ++i)
            {
                float y = origin.y + (1.f - (*steps[i] - valMin) / range) * size.y;
                float x = origin.x + (i + 0.5f) * barW;
                poly.push_back(ImVec2(x, y));
            }
        }
        dl->AddPolyline(poly.data(), (int)poly.size(),
                        IM_COL32(0, 200, 255, 220), 0, 1.5f);
    }

    // Node circles (centre of each bar at value height)
    {
        const float nodeRadius    = 4.f;
        const float nodeHitRadius = nodeRadius * 2.5f;
        for (int i = 0; i < nSteps; ++i)
        {
            const float cx = origin.x + (i + 0.5f) * barW;
            const float cy = origin.y + (1.f - (*steps[i] - valMin) / range) * size.y;

            const bool isDragging = isActive && drag.stepIdx == i;
            const bool isHovered  = !isActive && barUnderMouse == i;

            const ImU32 fill = isDragging  ? IM_COL32(255, 200,   0, 255)
                             : isHovered   ? IM_COL32(100, 180, 255, 255)
                                           : IM_COL32(160, 160, 160, 255);
            const ImU32 ring = isDragging  ? IM_COL32(255, 255, 100, 255)
                             : isHovered   ? IM_COL32(180, 220, 255, 255)
                                           : IM_COL32(100, 100, 100, 200);
            const float r = (isDragging || isHovered) ? nodeRadius + 1.5f : nodeRadius;

            dl->AddCircleFilled(ImVec2(cx, cy), r, fill);
            dl->AddCircle      (ImVec2(cx, cy), r, ring);
        }
    }

    // Cursor feedback
    if (mouseInWidget && !isActive)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    }

    // Border brightens on hover / active
    if (mouseInWidget || isActive)
    {
        dl->AddRect(origin, rectMax, IM_COL32(140, 140, 140, 255));
    }

    // Drag label
    if (isActive && drag.stepIdx >= 0)
    {
        char label[32];
        snprintf(label, sizeof(label), "S%d: %+.0f",
                 drag.stepIdx + 1, *steps[drag.stepIdx]);
        const float  fsz  = ImGui::GetFontSize() * 0.9f;
        const ImVec2 tpos = ImVec2(origin.x + 4.f,
                                   origin.y + size.y - fsz - 4.f);
        dl->AddText(ImGui::GetFont(), fsz, tpos,
                    IM_COL32(200, 200, 200, 200), label);
    }

    ImGui::PopID();
    return changed;
}

} // namespace ImStepLfo
