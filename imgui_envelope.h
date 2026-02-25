#pragma once
#include "imgui.h"
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

namespace ImEnvelope
{
struct DragState
{
    int    nodeIdx   = -1;
    ImVec2 startMouse =
    {0.f, 0.f};
    float  startLevel = 0.f;
    float  startTime  = 0.f;
};

// levels : array of nLevels pointers  (L0 .. Ln)
// times  : array of nTimes  pointers  (T1 .. Tn),  nTimes == nLevels - 1
// levelMin/levelMax : gui range for level axis
// timeMax           : max value for a single time segment
// sustainSegment    : when true, a fixed visual-only node is inserted between
//                     the second-to-last and last level node (same height as
//                     second-to-last), representing "key held" sustain.
// Returns true if any value was changed by the user.
inline bool EnvelopeWidget(
    const char* strId,
    float** levels, int nLevels,
    float** times,  int nTimes,
    float levelMin, float levelMax,
    float timeMax,
    ImVec2 size           = ImVec2(0.f, 120.f),
    bool   sustainSegment = false)
{
    IM_ASSERT(nLevels == nTimes + 1);

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

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImVec2 rectMax(origin.x + size.x, origin.y + size.y);
    const float levelRange = levelMax - levelMin;

    // When sustainSegment is true the effective time axis is widened by one
    // extra timeMax slot (the fixed sustain stretch between L(n-2) and L(n-1)).
    const float effectiveTotalTime = timeMax * (float)nTimes
                                   + (sustainSegment ? timeMax : 0.f);
    const float scale = (effectiveTotalTime > 0.f)
                            ? size.x / effectiveTotalTime : 0.f;

    // Background
    dl->AddRectFilled(origin, rectMax, IM_COL32(25, 25, 25, 255));
    dl->AddRect      (origin, rectMax, IM_COL32(80, 80, 80, 255));

    // Zero line
    const float zeroY = origin.y + (1.f - (-levelMin) / levelRange) * size.y;
    dl->AddLine(ImVec2(origin.x, zeroY), ImVec2(rectMax.x, zeroY),
                IM_COL32(60, 60, 60, 255));

    // Active node screen positions
    // When sustainSegment is true, the last node (L4) is shifted right by
    // one timeMax slot relative to the second-to-last node (L3).
    std::vector<ImVec2> nodePos(nLevels);
    {
        float accum = 0.f;
        for (int i = 0; i < nLevels; ++i)
        {
            if (sustainSegment && i == nLevels - 1)
            {
                accum += timeMax;   // skip over the sustain slot
            }
            float yNorm = 1.f - (*levels[i] - levelMin) / levelRange;
            nodePos[i] = ImVec2(origin.x + accum * scale,
                                origin.y + yNorm * size.y);
            if (i < nTimes)
            {
                accum += *times[i];
            }
        }
    }

    // Sustain node (display-only)
    // Sits between nodePos[nLevels-2] and nodePos[nLevels-1],
    // always at the same height as nodePos[nLevels-2].
    ImVec2 sustainPos = {};
    if (sustainSegment)
    {
        sustainPos = ImVec2(nodePos[nLevels - 2].x + timeMax * scale,
                            nodePos[nLevels - 2].y);
    }

    // Polyline
    {
        std::vector<ImVec2> poly;
        poly.reserve(nLevels + 1);
        for (int i = 0; i < nLevels - 1; ++i)
        {
            poly.push_back(nodePos[i]);
        }
        if (sustainSegment)
        {
            poly.push_back(sustainPos);
        }
        poly.push_back(nodePos[nLevels - 1]);
        dl->AddPolyline(poly.data(), (int)poly.size(),
                        IM_COL32(0, 180, 255, 200), 0, 2.f);
    }

    // Drag state (keyed by widget ID)
    static std::map<ImGuiID, DragState> sDragStates;
    DragState& drag = sDragStates[wid];

    const ImVec2 mousePos      = ImGui::GetMousePos();
    const float  nodeRadius    = 5.f;
    const float  nodeHitRadius = nodeRadius * 2.5f;

    // On first active frame: find nearest active node (sustain node excluded)
    if (ImGui::IsItemActivated())
    {
        float minDist2 = nodeHitRadius * nodeHitRadius;
        drag.nodeIdx   = -1;
        for (int i = 0; i < nLevels; ++i)
        {
            float dx = mousePos.x - nodePos[i].x;
            float dy = mousePos.y - nodePos[i].y;
            float d2 = dx * dx + dy * dy;
            if (d2 < minDist2)
            {
                minDist2        = d2;
                drag.nodeIdx    = i;
                drag.startMouse = mousePos;
                drag.startLevel = *levels[i];
                drag.startTime  = (i > 0) ? *times[i - 1] : 0.f;
                // For the last node, T4 is times[nTimes-1]
                if (i == nLevels - 1 && sustainSegment)
                {
                     drag.startTime = *times[nTimes - 1];
                }
            }
        }
    }

    // While active: update the dragged node
    if (isActive && drag.nodeIdx >= 0)
    {
        const ImVec2 delta(mousePos.x - drag.startMouse.x,
                           mousePos.y - drag.startMouse.y);

        // Vertical -> level
        float newLevel = std::round(std::clamp(
            drag.startLevel - (delta.y / size.y) * levelRange,
            levelMin, levelMax));
        if (newLevel != *levels[drag.nodeIdx])
        {
            *levels[drag.nodeIdx] = newLevel;
            changed = true;
        }

        // Horizontal -> preceding time (not for node 0)
        if (drag.nodeIdx > 0)
        {
            int timeIdx = drag.nodeIdx - 1;
            // Last node's horizontal drag controls times[nTimes-1] (= T4)
            if (sustainSegment && drag.nodeIdx == nLevels - 1)
            {
                 timeIdx = nTimes - 1;
            }
            float newTime = std::round(std::clamp(
                drag.startTime + (delta.x / size.x) * effectiveTotalTime,
                0.f, timeMax));
            if (newTime != *times[timeIdx])
            {
                *times[timeIdx] = newTime;
                changed = true;
            }
        }
    }

    // Hover / cursor feedback
    const bool mouseInWidget =
        mousePos.x >= origin.x && mousePos.x <= rectMax.x &&
        mousePos.y >= origin.y && mousePos.y <= rectMax.y;

    bool anyNodeHovered = false;
    for (int i = 0; i < nLevels; ++i)
    {
        float dx = mousePos.x - nodePos[i].x;
        float dy = mousePos.y - nodePos[i].y;
        if ((dx * dx + dy * dy) < nodeHitRadius * nodeHitRadius)
        {
            anyNodeHovered = true;
            break;
        }
    }
    if (mouseInWidget && !isActive)
    {
        ImGui::SetMouseCursor(anyNodeHovered ? ImGuiMouseCursor_ResizeAll
                                             : ImGuiMouseCursor_Hand);
    }

    // Widget border brightens on hover
    if (mouseInWidget || isActive)
    {
        dl->AddRect(origin, rectMax, IM_COL32(140, 140, 140, 255));
    }
    // Draw sustain node first (below active nodes)
    if (sustainSegment)
    {
        dl->AddCircle(sustainPos, nodeRadius, IM_COL32(120, 120, 120, 180));
    }

    // Draw active nodes (on top of sustain)
    for (int i = 0; i < nLevels; ++i)
    {
        const bool nodeActive  = isActive && drag.nodeIdx == i;
        bool       nodeHovered = false;
        if (!isActive && mouseInWidget)
        {
            float dx = mousePos.x - nodePos[i].x;
            float dy = mousePos.y - nodePos[i].y;
            nodeHovered = (dx * dx + dy * dy) < nodeHitRadius * nodeHitRadius;
        }
        const ImU32 fill = nodeActive
            ? IM_COL32(255, 200,   0, 255)
            : nodeHovered ? IM_COL32(100, 180, 255, 255)
                          : IM_COL32(160, 160, 160, 255);
        const ImU32 ring = nodeActive  ? IM_COL32(255, 255, 100, 255)
                         : nodeHovered ? IM_COL32(180, 220, 255, 255)
                                       : IM_COL32(100, 100, 100, 200);
        const float r = nodeHovered || nodeActive ? nodeRadius + 1.5f : nodeRadius;
        dl->AddCircleFilled(nodePos[i], r, fill);
        dl->AddCircle      (nodePos[i], r, ring);
    }

    // Drag label (bottom-left, small)
    if (isActive && drag.nodeIdx >= 0)
    {
        const int idx = drag.nodeIdx;
        char label[64];
        if (idx > 0)
        {
            const int tIdx = (sustainSegment && idx == nLevels - 1)
                                 ? nTimes - 1 : idx - 1;
            snprintf(label, sizeof(label), "L%d: %.0f  T%d: %.0f",
                     idx, *levels[idx], tIdx + 1, *times[tIdx]);
        }
        else
        {
            snprintf(label, sizeof(label), "L0: %.0f", *levels[0]);
        }
        const float  fontSize = ImGui::GetFontSize() * 0.9f;
        const ImVec2 textPos(origin.x + 4.f,
                             origin.y + size.y - fontSize - 4.f);
        dl->AddText(ImGui::GetFont(), fontSize, textPos,
                    IM_COL32(200, 200, 200, 200), label);
    }

    ImGui::PopID();
    return changed;
}

} // namespace ImEnvelope
