#include "imgui.h"
#include "imgui-knobs.h"
#include "imcmd_command_palette.h"
#include "imsearch.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sol/sol.hpp>
#include <iostream>
#include <rtmidi/RtMidi.h>
#include <string>
#include "ParameterDef.h"
#include <vector>
#include <tuple>

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

// BEGIN: LUA to DEF. TODO Refactor into separate file
template<typename T>
T require_key(sol::table tbl, const std::string& key) {
    sol::optional<T> val = tbl[key];
    if (!val) {
        throw std::runtime_error("Lua table is missing required key: " + key);
    }
    return val.value();
}

template<typename T>
T optional_key(sol::table tbl, const std::string& key, const T& defaultValue) {
    sol::optional<T> val = tbl[key];
    if (!val.has_value()) {
       return defaultValue;
    }
    return val.value();
}

std::string getDefaultValue(const ParameterDef::SelectionOptions &options, int defaultValue)
{
    const auto& it = options.find(defaultValue);
    if (it == options.end())
    {
        return std::string();
    }
    return it->second;
}

SectionDef getSection(const sol::table &lua_table)
{
    SectionDef sectionDef;
    sectionDef.name = require_key<std::string>(lua_table, "name");
    if (lua_table["params"] != sol::nil) 
    {
        sol::table params = lua_table["params"];
        for(auto &luaParamPair : params)
        {
            ParameterDef param;
            auto luaParam = luaParamPair.second.as<sol::table>();
            sol::object paramName = luaParam["name"];
            sol::object paramId = luaParam["id"];
            param.name = paramName.as<ParameterDef::FStringGetter>();
            param.id = paramId.as<std::string>();
            param.type = require_key<std::string>(luaParam, "type");
            param.setValue = require_key<ParameterDef::FSetValue>(luaParam, "setValue");
            param.min = optional_key(luaParam, "min", param.min);
            param.max = optional_key(luaParam, "max", param.max);
            param.format = optional_key(luaParam, "format", param.format);
            param.value = optional_key(luaParam, "default", param.min ? param.min() : 0);
            param.toI7Value = optional_key<ParameterDef::FToI7Value>(luaParam, "toI7Value", nullptr);
            param.options = optional_key<ParameterDef::SelectionOptions>(luaParam, "options", ParameterDef::SelectionOptions());
            if (!param.options.empty())
            {
                param.stringValue = getDefaultValue(param.options, param.value);
            }
            sectionDef.params.emplace_back(param);
        }
    }
    if(lua_table["sub"] != sol::nil)
    {
        sol::table luaSubSections = lua_table["sub"];
        for(const auto& luaSubSectionObj : luaSubSections)
        {
            sol::table luaSubSection = luaSubSectionObj.second;
            auto subSection = getSection(luaSubSection);
            sectionDef.subSections.push_back(subSection);
        }
    }
    if(lua_table["isOpen"] != sol::nil)
    {
        sectionDef.isOpen = lua_table["isOpen"];
    }
    return sectionDef;
}

SectionDef::NamedSections getDefs(const sol::state &lua)
{
    SectionDef::NamedSections result;
    sol::table mainSections = lua["Main"];
    for(auto &luaSectionPair : mainSections)
    {
        sol::table luaSection = luaSectionPair.second.as<sol::table>();
        auto sectionDef = getSection(luaSection);
        result.insert({luaSectionPair.first.as<std::string>(), sectionDef});
    }
    return result;
}
// END: Lua to Def

void valueChanged(const sol::state &lua, RtMidiOut &midiOut, const ParameterDef& paramDef)
{
    try 
    {
        int i7Value = (int)paramDef.value;
        if (paramDef.toI7Value)
        {
            i7Value = paramDef.toI7Value(paramDef.value);
        }
        auto sysex = paramDef.setValue(i7Value);
        midiOut.sendMessage(&sysex);
    } catch(const sol::error error)
    {
        std::cerr << error.what() << std::endl;
    }
}

struct I7Ed 
{
    RtMidiIn  midiIn;
    RtMidiOut midiOut;
    Args args;
    sol::state lua;
};

void renderCombo(ParameterDef& param, I7Ed &ed)
{
    if (ImGui::BeginCombo(param.name().c_str(), param.stringValue.c_str()))
    {
        if (ImSearch::BeginSearch())
        {
            ImSearch::SearchBar();

            for (const auto& [value, label] : param.options)
            {

                ImSearch::SearchableItem(label.c_str(),
                    [&](const char* name)
                    {
                        const bool isSelected = name == param.stringValue;
                        if (ImGui::Selectable(name, isSelected))
                        {
                            param.stringValue = name;
                            param.value = value;
                            valueChanged(ed.lua, ed.midiOut, param);
                        }
                    });
            }
            ImSearch::EndSearch();
        }
        ImGui::EndCombo();
    }
}

void renderSection(SectionDef &section, I7Ed &ed)
{
    for(auto &param : section.params)
    {
        if (param.type == PARAM_TYPE_RANGE)
        {
            if (ImGuiKnobs::Knob(param.name().c_str(), &param.value, param.min(), param.max(), 0.0f, param.format.c_str(), ImGuiKnobVariant_Tick, 0 , ImGuiKnobFlags_AlwaysClamp)) 
            {
                valueChanged(ed.lua, ed.midiOut, param);
            }
        }
        else if (param.type == PARAM_TYPE_SELECTION)
        {
            renderCombo(param, ed);
        }
        else 
        {
            std::cerr << "unknown param type: '" << param.type << "'" << std::endl; 
        }

    }
    return;
}

int main(int argc, const char** args)
{
    I7Ed ed;
    ed.args = parseArguments(argc, args);
    if (ed.args.printHelp)
    {
        std::cout << "Allowed options:\n" 
                  << "\t--inputs\n"
                  << "\t--outputs\n"
                  << "\t--lua-main\n"
                  << "\t--in-portnr\n"
                  << "\t--out-portnr\n"
                  << std::endl;
        return 0;
    }
    if (ed.args.listInputs)
    {
        std::cout << "Inputs:"  << std::endl;
        printMidiIo<RtMidiIn>();
    }
    if (ed.args.listOutputs)
    {
        std::cout << "Outputs:"  << std::endl;
        printMidiIo<RtMidiOut>();
    }
    if (ed.args.listOutputs || ed.args.listInputs)
    {
        return 0;
    }

    if (ed.args.inPortNr >= 0)
    {
        openPort(ed.midiIn, (size_t)ed.args.inPortNr);
    }
    if (ed.args.outPortNr >= 0)
    {
        openPort(ed.midiOut, (size_t)ed.args.outPortNr);
    }
    
    const char *luaFile = ed.args.mainLuaFilePath.empty() 
        ?  "./lua/main.lua" 
        : ed.args.mainLuaFilePath.c_str();

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
    ImSearch::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); 

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	ed.lua.open_libraries();
    ed.lua.script_file(luaFile);

    auto sections = getDefs(ed.lua);
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

    std::vector<std::string> demoCombo({"people","history","way","art","world","information","map","two","family"});

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
                renderSection(section, ed);
                for(auto &subSection : section.subSections)
                {
                    renderSection(subSection, ed);
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

    ImCmd::DestroyContext();
    ImSearch::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    if (ed.midiIn.isPortOpen())
    {
         ed.midiIn.closePort();
    }
    if (ed.midiOut.isPortOpen())
    {
        ed.midiOut.closePort();
    }
    return 0;
}