#include "imgui.h"
#include "imgui-knobs.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sol/sol.hpp>
#include <iostream>
#include <rtmidi/RtMidi.h>


struct Args 
{
    bool listOutputs = false;
    bool listInputs = false;
    bool printHelp = false;
};

Args parseArguments(int argc, const char **argv)
{
    Args result;
    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if (std::string(arg) == "--outputs")
        {
            result.listOutputs = true;
        }
        if (std::string(arg) == "--inputs")
        {
            result.listInputs = true;
        }
        if (std::string(arg) == "--help")
        {
            result.printHelp = true;
        }

    }
    return result;
}


template<class TMidiIO>
void printMidiIo()
{
    TMidiIO midiIo;
    auto nOutputs = midiIo.getPortCount();
    for (size_t idx = 0; idx < nOutputs; ++idx)
    {
        std::cout << (idx) << ": " << midiIo.getPortName(idx) << std::endl;
    }
}


int main(int argc, const char** args)
{

    auto parsedArgs = parseArguments(argc, args);
    if (parsedArgs.printHelp)
    {
        std::cout << "Allowed options:\n" 
                  << "\t--inputs\n"
                  << "\t--outputs\n"
                  << std::endl;
        return 0;
    }
    if (parsedArgs.listInputs)
    {
        std::cout << "Inputs:"  << std::endl;
        printMidiIo<RtMidiIn>();
    }
    if (parsedArgs.listOutputs)
    {
        std::cout << "Outputs:"  << std::endl;
        printMidiIo<RtMidiOut>();
    }
    if (parsedArgs.listOutputs || parsedArgs.listInputs)
    {
        return 0;
    }
    const char *luaFile = "./lua/main.lua";
    if (!glfwInit())
    {
        return -1;
    }

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