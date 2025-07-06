#include "imgui.h"
#include "imgui-knobs.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sol/sol.hpp>
#include <iostream>

int main(int argc, const char** args)
{
    if (argc < 2) 
    {
        std::cout << "missing lua file" << std::endl;
        return -1;
    }
    const char *luaFile = args[1];

    if (!glfwInit())
    {
        return -1;
    }

    // OpenGL-Kontext Version 3.0+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello, ImGui!", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    sol::state lua;
	lua.open_libraries(sol::lib::base);
    lua.script_file(luaFile);

    int x = lua["X"];
    std::cout << x << std::endl;

    std::string message = lua["Main"]["message"];

    float value = 0;
    float value2 = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello, world!");
        ImGui::TextUnformatted(message.c_str());

       
        if (ImGuiKnobs::Knob("Volume", &value, -6.0f, 6.0f, 0.1f, "%.1fdB", ImGuiKnobVariant_Tick))
        {
            // value was changed
        }
        
        if (ImGuiKnobs::Knob("Volume2", &value2, -6.0f, 6.0f, 0.1f, "%.1fdB", ImGuiKnobVariant_Tick))
        {
            // value was changed
        }

        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}