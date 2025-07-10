#include "imgui.h"
#include "imgui-knobs.h"
#include "imcmd_command_palette.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sol/sol.hpp>
#include <iostream>
#include <rtmidi/RtMidi.h>
#include <string>
#include "ParameterDef.h"
#include <vector>

struct Args 
{
    bool listOutputs = false;
    bool listInputs = false;
    bool printHelp = false;
    std::string mainLuaFilePath;
    int inPortNr = -1;
    int outPortNr = -1;
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
        else if (std::string(arg) == "--inputs")
        {
            result.listInputs = true;
        }
        else if (std::string(arg) == "--lua-main")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.mainLuaFilePath = std::string(argv[i]);
            continue;
        }
        else if (std::string(arg) == "--in-portnr")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.inPortNr = (int)atoi(argv[i]);
            continue;
        }
        else if (std::string(arg) == "--out-portnr")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.outPortNr = (int)atoi(argv[i]);
            continue;
        }
        else if (std::string(arg) == "--help")
        {
            result.printHelp = true;
        }
        else 
        {
            std::cout << "unkown argument: " << arg << std::endl;
            exit(-1);
        }
    }
    return result;
}



template<class TMidiIO>
void printMidiIo()
{
    TMidiIO midiIo;
    auto nIos = midiIo.getPortCount();
    for (size_t idx = 0; idx < nIos; ++idx)
    {
        std::cout << (idx) << ": " << midiIo.getPortName(idx) << std::endl;
    }
}

template<class TMidiIO>
void openPort(TMidiIO &port, size_t portNr)
{
    auto nIos = port.getPortCount();
    if (portNr >= nIos)
    {
        std::cerr << "invalid port number: " << portNr << std::endl;
        exit(1);
    } 
    port.openPort(portNr);
    std::cout << "open: (" << portNr << ") " << port.getPortName(portNr) << std::endl;
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

void valueChanged(sol::state &lua, RtMidiOut &midiOut, const ParameterDef& def)
{
    sol::function create_sysex = lua["CreateSysexMessage"];
    sol::protected_function_result result = create_sysex(def.id, def.value);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "Lua error: " << err.what() << "\n";
        return;
    }
    sol::table sysexTable = result;
    auto sysex = sysexTable.as<std::vector<unsigned char>>();
    midiOut.sendMessage(&sysex);
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
                  << "\t--in-portnr\n"
                  << "\t--in-portnr\n"
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

    RtMidiIn midiIn;
    RtMidiOut midiOut;

    if (parsedArgs.inPortNr >= 0)
    {
        openPort(midiIn, (size_t)parsedArgs.inPortNr);
    }
    if (parsedArgs.outPortNr >= 0)
    {
        openPort(midiOut, (size_t)parsedArgs.outPortNr);
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
    glfwSwapInterval(0); // V-Sync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImCmd::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    sol::state lua;
	lua.open_libraries();
    lua.script_file(luaFile);

    auto sections = getDefs(lua);
    bool show_command_palette = false;

    for(auto &section : sections)
    {
        ImCmd::Command cmd;
        cmd.Name = std::string("open ") + section.second.name;
        cmd.InitialCallback = [&section]() {
            section.second.isOpen = true;
        };
        ImCmd::AddCommand(std::move(cmd));
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P)) {
            show_command_palette = !show_command_palette;
        }
        if (show_command_palette) {
            ImCmd::CommandPaletteWindow("CommandPalette", &show_command_palette);
        }
        for(auto &sectionPair : sections)
        {
            auto& section = sectionPair.second;
            if (!section.isOpen)
            {
                continue;
            }
            if (ImGui::Begin(section.name.c_str(), &section.isOpen))
            {
                for(auto &param : section.params)
                {
                    // TODO: make min, max part of param def
                    if (ImGuiKnobs::Knob(param.name.c_str(), &param.value, 0.0f, 100.0f, 1.0f, "%.1fdB", ImGuiKnobVariant_Tick)) {
                        valueChanged(lua, midiOut, param);
                    }
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
    if (midiIn.isPortOpen())
    {
        midiIn.closePort();
    }
    if (midiOut.isPortOpen())
    {
        midiOut.closePort();
    }
    return 0;
}