#pragma once
#include "imgui.h"
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

namespace ImEnvelope {

struct DragState {
    int  nodeIdx   = -1;
    ImVec2 startMouse = {0.f, 0.f};
    float  startLevel = 0.f;
    float  startTime  = 0.f;
};

// levels : array of nLevels pointers  (L0 .. Ln)
// times  : array of nTimes  pointers  (T1 .. Tn),  nTimes == nLevels - 1
// levelMin/levelMax : gui range for level axis
// timeMax           : max value for a single time segment
// Returns true if any value was changed by the user.
inline bool EnvelopeWidget(
    const char* strId,
    float** levels, int nLevels,
    float** times,  int nTimes,
    float levelMin, float levelMax,
    float timeMax,
    ImVec2 size = ImVec2(0.f, 120.f))
{
    IM_ASSERT(nLevels == nTimes + 1);

    bool changed = false;
    ImGui::PushID(strId);

    if (size.x <= 0.f) size.x = ImGui::GetContentRegionAvail().x;

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("canvas", size);
    const bool isActive  = ImGui::IsItemActive();
    const ImGuiID wid    = ImGui::GetItemID();

    ImDrawList* dl       = ImGui::GetWindowDrawList();
    ImVec2 rectMax(origin.x + size.x, origin.y + size.y);
    const float levelRange    = levelMax - levelMin;
    const float totalMaxTime  = timeMax * (float)nTimes;

    // ── Background ──────────────────────────────────────────────────────────
    dl->AddRectFilled(origin, rectMax, IM_COL32(25, 25, 25, 255));
    dl->AddRect      (origin, rectMax, IM_COL32(80, 80, 80, 255));

    // Zero line
    const float zeroY = origin.y + (1.f - (-levelMin) / levelRange) * size.y;
    dl->AddLine(ImVec2(origin.x, zeroY), ImVec2(rectMax.x, zeroY),
                IM_COL32(60, 60, 60, 255));

    // ── Node screen positions ────────────────────────────────────────────────
    std::vector<ImVec2> nodePos(nLevels);
    {
        float accum = 0.f;
        for (int i = 0; i < nLevels; ++i) {
            float xNorm = (totalMaxTime > 0.f) ? (accum / totalMaxTime) : 0.f;
            float yNorm = 1.f - (*levels[i] - levelMin) / levelRange;
            nodePos[i]  = ImVec2(origin.x + xNorm * size.x,
                                 origin.y + yNorm * size.y);
            if (i < nTimes) accum += *times[i];
        }
    }

    // Polyline
    dl->AddPolyline(nodePos.data(), nLevels,
                    IM_COL32(0, 180, 255, 200), 0, 2.f);

    // ── Drag state (keyed by widget ID) ─────────────────────────────────────
    static std::map<ImGuiID, DragState> sDragStates;
    DragState& drag = sDragStates[wid];

    const ImVec2 mousePos      = ImGui::GetMousePos();
    const float  nodeRadius    = 5.f;
    const float  nodeHitRadius = nodeRadius * 2.5f;

    // On first active frame: find the nearest node
    if (ImGui::IsItemActivated()) {
        float minDist2 = nodeHitRadius * nodeHitRadius;
        drag.nodeIdx   = -1;
        for (int i = 0; i < nLevels; ++i) {
            float dx = mousePos.x - nodePos[i].x;
            float dy = mousePos.y - nodePos[i].y;
            float d2 = dx * dx + dy * dy;
            if (d2 < minDist2) {
                minDist2        = d2;
                drag.nodeIdx    = i;
                drag.startMouse = mousePos;
                drag.startLevel = *levels[i];
                drag.startTime  = (i > 0) ? *times[i - 1] : 0.f;
            }
        }
    }

    // While active: update the dragged node
    if (isActive && drag.nodeIdx >= 0) {
        const ImVec2 delta(mousePos.x - drag.startMouse.x,
                           mousePos.y - drag.startMouse.y);

        // Vertical → level
        float newLevel = std::round(std::clamp(
            drag.startLevel - (delta.y / size.y) * levelRange,
            levelMin, levelMax));
        if (newLevel != *levels[drag.nodeIdx]) {
            *levels[drag.nodeIdx] = newLevel;
            changed = true;
        }

        // Horizontal → preceding time (not for node 0)
        if (drag.nodeIdx > 0) {
            float newTime = std::round(std::clamp(
                drag.startTime + (delta.x / size.x) * totalMaxTime,
                0.f, timeMax));
            if (newTime != *times[drag.nodeIdx - 1]) {
                *times[drag.nodeIdx - 1] = newTime;
                changed = true;
            }
        }
    }

    // ── Draw nodes ───────────────────────────────────────────────────────────
    const bool mouseInWidget =
        mousePos.x >= origin.x && mousePos.x <= rectMax.x &&
        mousePos.y >= origin.y && mousePos.y <= rectMax.y;

    for (int i = 0; i < nLevels; ++i) {
        const bool nodeActive  = isActive && drag.nodeIdx == i;
        bool       nodeHovered = false;
        if (!isActive && mouseInWidget) {
            float dx = mousePos.x - nodePos[i].x;
            float dy = mousePos.y - nodePos[i].y;
            nodeHovered = (dx * dx + dy * dy) < nodeHitRadius * nodeHitRadius;
        }

        const ImU32 color = nodeActive
            ? IM_COL32(255, 220,   0, 255)
            : nodeHovered
                ? IM_COL32(200, 200, 255, 255)
                : IM_COL32(200, 200, 200, 255);

        dl->AddCircleFilled(nodePos[i], nodeRadius, color);
        dl->AddCircle      (nodePos[i], nodeRadius, IM_COL32(100, 100, 100, 200));
    }

    ImGui::PopID();
    return changed;
}

} // namespace ImEnvelope
