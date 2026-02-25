#pragma once
#include "imgui.h"
#include <atomic>
#include <chrono>
#include <algorithm>

// Draws two small activity LEDs (RX red, TX green) in the bottom-right corner.
// sendTimeNs / recvTimeNs : nanoseconds since epoch of last activity
//                           (set via Clock::now().time_since_epoch().count())
// Call once per frame between NewFrame() and Render().
inline void renderMidiActivityLeds(
    int displayW, int displayH,
    const std::atomic<int64_t>& sendTimeNs,
    const std::atomic<int64_t>& recvTimeNs,
    ImVec2 scrollOfs = {0.f, 0.f})
{
    using Clock = std::chrono::steady_clock;
    constexpr float kFade    = 0.15f;  // fade-out duration in seconds
    constexpr float kRadius  = 5.f;
    constexpr float kMargin  = 14.f;
    constexpr float kSpacing = 20.f;   // center-to-center distance

    ImDrawList* dl = ImGui::GetForegroundDrawList();

    auto brightness = [](const std::atomic<int64_t>& ns) -> float 
    {
        const int64_t t = ns.load(std::memory_order_relaxed);
        if (t == 0)
        {
            return 0.f;
        }
        using namespace std::chrono;
        const auto tp  = Clock::time_point(nanoseconds(t));
        const float age = duration<float>(Clock::now() - tp).count();
        return std::max(0.f, 1.f - age / kFade);
    };

    const float sendB = brightness(sendTimeNs);
    const float recvB = brightness(recvTimeNs);

    // RX (red) left of TX (green)
    const ImVec2 rxPos((float)displayW - kMargin - kSpacing + scrollOfs.x, (float)displayH - kMargin + scrollOfs.y);
    const ImVec2 txPos((float)displayW - kMargin             + scrollOfs.x, (float)displayH - kMargin + scrollOfs.y);

    auto drawLed = [&](ImVec2 pos, float b, ImVec4 active, ImVec4 dim) 
    {
        // soft glow halo when active
        if (b > 0.01f) 
        {
            dl->AddCircleFilled(pos, kRadius * 2.5f,
                IM_COL32((int)(active.x * 255), (int)(active.y * 255),
                         (int)(active.z * 255), (int)(b * 55)));
        }
        // LED body — interpolate between dim and active colour
        const ImVec4 col(
            dim.x + (active.x - dim.x) * b,
            dim.y + (active.y - dim.y) * b,
            dim.z + (active.z - dim.z) * b,
            1.f);
        dl->AddCircleFilled(pos, kRadius,  ImGui::ColorConvertFloat4ToU32(col));
        dl->AddCircle      (pos, kRadius,  IM_COL32(55, 55, 55, 220));
    };

    drawLed(rxPos, recvB,
            ImVec4(1.f,  0.15f, 0.15f, 1.f),   // active: red
            ImVec4(0.14f, 0.03f, 0.03f, 1.f));  // dim:    dark red

    drawLed(txPos, sendB,
            ImVec4(0.15f, 1.f,  0.15f, 1.f),   // active: green
            ImVec4(0.03f, 0.14f, 0.03f, 1.f));  // dim:    dark green

    // small labels above the LEDs
    ImFont* font      = ImGui::GetFont();
    const float fsz   = ImGui::GetFontSize() * 0.7f;
    const float labelY = rxPos.y - kRadius - fsz - 2.f;  // rxPos already includes scrollOfs.y
    const ImU32 labelColor = IM_COL32(130, 130, 130, 200);

    dl->AddText(font, fsz, ImVec2(rxPos.x - fsz * 0.75f, labelY), labelColor, "RX");
    dl->AddText(font, fsz, ImVec2(txPos.x - fsz * 0.60f, labelY), labelColor, "TX");
}
