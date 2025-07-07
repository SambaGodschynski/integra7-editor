#include "imgui.h"
#include "imgui-knobs.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sol/sol.hpp>
#include <iostream>
#include <rtmidi/RtMidi.h>
#include <string>
#include "ParameterDef.h"

struct Args 
{
    bool listOutputs = false;
    bool listInputs = false;
    bool printHelp = false;
    std::string mainLuaFilePath;
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
            continue;
        }
        if (std::string(arg) == "--inputs")
        {
            result.listInputs = true;
            continue;
        }
        if (std::string(arg) == "--lua-main")
        {
            ++i;
            result.mainLuaFilePath = std::string(argv[i]);
            continue;
        }
        if (std::string(arg) == "--help")
        {
            result.printHelp = true;
            continue;
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


SectionDef::Sections getDefs(sol::state &lua)
{
    SectionDef::Sections result;
    sol::table mainSections = lua["Main"];
    for(auto &luaSectionPair : mainSections)
    {
        SectionDef sectionDef;
        auto luaSection = luaSectionPair.second.as<sol::table>();
        sol::object name = luaSection["name"];
        sectionDef.name = name.as<std::string>();
        sol::table params = luaSection["params"];
        for(auto &luaParamPair : params)
        {
            ParameterDef param;
            auto luaParam = luaParamPair.second.as<sol::table>();
            sol::object paramName = luaParam["name"];
            sol::object paramId = luaParam["id"];
            param.name = paramName.as<std::string>();
            param.id = paramId.as<std::string>();
            sectionDef.params.push_back(param);
        }
        result.insert({luaSectionPair.first.as<std::string>(), sectionDef});
    }
    return result;
}

int main(int argc, const char** args)
{
    auto parsedArgs = parseArguments(argc, args);
    if (parsedArgs.printHelp)
    {
        std::cout << "Allowed options:\n" 
                  << "\t--inputs\n"
                  << "\t--outputs\n"
                  << "\t--lua-main\n"
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
    const char *luaFile = parsedArgs.mainLuaFilePath.empty() 
        ?  "./lua/main.lua" 
        : parsedArgs.mainLuaFilePath.c_str();

    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "i7Ed", NULL, NULL);
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

    auto sections = getDefs(lua);


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for(auto &sectionPair : sections)
        {
            auto& section = sectionPair.second;
            ImGui::Begin(section.name.c_str());
            for(auto &param : section.params)
            {
                if (ImGuiKnobs::Knob(param.name.c_str(), &param.value, -6.0f, 6.0f, 0.1f, "%.1fdB", ImGuiKnobVariant_Tick)) {
                    // value was changed
                }
            }

            ImGui::End();
        }

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