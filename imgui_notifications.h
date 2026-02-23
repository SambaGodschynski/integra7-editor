#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <cstdio>

struct Notification
{
    std::string message;
    ImVec4      color    = ImVec4(1.f, 0.4f, 0.2f, 1.f);
    float       duration = 3.0f;   // total seconds visible
    float       fadeAt   = 2.0f;   // start fading after this many seconds
    std::chrono::steady_clock::time_point createdAt;
    int         id       = 0;
};

// Thread-safe notification queue.
// Call push() from any thread, render() once per frame on the main thread.
struct NotificationQueue
{
    void push(const std::string& message,
              ImVec4 color       = ImVec4(1.f, 0.4f, 0.2f, 1.f),
              float  duration    = 3.0f,
              float  fadeAt      = 2.0f)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back({message, color, duration, fadeAt,
                          std::chrono::steady_clock::now(), nextId_++});
    }

    // Draws all active notifications stacked in the bottom-right corner.
    // Call between ImGui::NewFrame() and ImGui::Render().
    void render(int displayW, int displayH)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Remove fully expired entries
        queue_.erase(
            std::remove_if(queue_.begin(), queue_.end(), [](const Notification& n) {
                return age(n) >= n.duration;
            }),
            queue_.end());

        constexpr ImGuiWindowFlags kFlags =
            ImGuiWindowFlags_NoDecoration    | ImGuiWindowFlags_NoNav         |
            ImGuiWindowFlags_NoMove          | ImGuiWindowFlags_NoInputs      |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

        float offsetY = 10.f;
        for (auto& n : queue_) 
        {
            const float a = alpha(n);

            ImGui::SetNextWindowBgAlpha(0.8f * a);
            ImGui::SetNextWindowPos(
                ImVec2((float)displayW - 10.f, offsetY),
                ImGuiCond_Always, ImVec2(1.f, 0.f));

            char wndId[32];
            std::snprintf(wndId, sizeof(wndId), "##notif_%d", n.id);

            if (ImGui::Begin(wndId, nullptr, kFlags)) 
            {
                ImGui::TextColored(
                    ImVec4(n.color.x, n.color.y, n.color.z, a),
                    "%s", n.message.c_str());
                offsetY += ImGui::GetWindowSize().y + 5.f;
            }
            ImGui::End();
        }
    }

private:
    static float age(const Notification& n) 
    {
        return std::chrono::duration<float>(
            std::chrono::steady_clock::now() - n.createdAt).count();
    }
    static float alpha(const Notification& n) 
    {
        const float e = age(n);
        if (e < n.fadeAt) 
        {
            return 1.0f;
        }
        return std::max(0.f, 1.f - (e - n.fadeAt) / (n.duration - n.fadeAt));
    }

    std::mutex            mutex_;
    std::vector<Notification> queue_;
    int                   nextId_ = 0;
};
