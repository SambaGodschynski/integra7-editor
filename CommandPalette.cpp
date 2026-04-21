#include "CommandPalette.h"
#include "imcmd_command_palette.h"
#include <set>
#include <unordered_map>
#include <string>

namespace
{
    constexpr float HighlightSeconds = 15.0f;
}

void setupCommandPalette(SectionDef::NamedSections& sections, I7Ed& ed)
{
    for (auto& section : sections)
    {
        if (section.second.hideFromPalette) { continue; }
        ImCmd::Command cmd;
        cmd.Name = std::string("open ") + section.second.name;
        cmd.InitialCallback = [&section]()
        {
            section.second.isOpen = true;
        };
        ImCmd::AddCommand(std::move(cmd));
    }

    struct NavInfo
    {
        SectionDef* opener;
        std::string tabLabel;
        std::string accordionLabel;
    };
    std::unordered_map<std::string, NavInfo> sectionNavInfo;
    for (auto& [tKey, tSec] : sections)
    {
        if (tSec.tabs.empty()) { continue; }
        if (!tSec.tabCommonKey.empty())
        {
            sectionNavInfo[tSec.tabCommonKey] = { &sections.at(tKey), "", "" };
        }
        for (const auto& tab : tSec.tabs)
        {
            for (const auto& ref : tab.sectionKeys)
            {
                sectionNavInfo[ref.key] = { &sections.at(tKey), tab.label, ref.accordionLabel };
            }
        }
    }

    std::set<std::string> seen;
    auto addParamCmds = [&](const SectionDef& sec, SectionDef* opener,
                            const std::string& tabLabel, const std::string& accordionLabel)
    {
        for (auto* param : sec.params)
        {
            if (!param) { continue; }
            const std::string pname = param->name();
            if (pname == HIDDEN_PARAM_NAME) { continue; }
            std::string cmdName = "? " + pname + " (" + opener->name + ")";
            if (!seen.insert(cmdName).second) { continue; }
            ImCmd::Command cmd;
            cmd.Name = std::move(cmdName);
            const std::string paramId = param->id;
            cmd.InitialCallback = [opener, paramId, tabLabel, accordionLabel, &ed]()
            {
                opener->isOpen = true;
                ed.search.highlightParamId       = paramId;
                ed.search.highlightTimer         = HighlightSeconds;
                ed.search.navigateOpenerName     = opener->name;
                ed.search.navigateTabLabel       = tabLabel;
                ed.search.navigateAccordionLabel = accordionLabel;
            };
            ImCmd::AddCommand(std::move(cmd));
        }
    };

    for (auto& [key, section] : sections)
    {
        std::string tabLabel, accordionLabel;
        SectionDef* opener = &section;
        if (section.hideFromPalette)
        {
            auto it = sectionNavInfo.find(key);
            if (it != sectionNavInfo.end())
            {
                opener         = it->second.opener;
                tabLabel       = it->second.tabLabel;
                accordionLabel = it->second.accordionLabel;
            }
        }
        addParamCmds(section, opener, tabLabel, accordionLabel);
        for (auto& sub : section.subSections)
        {
            const std::string subAccordion = section.accordion ? sub.name : accordionLabel;
            addParamCmds(sub, opener, tabLabel, subAccordion);
        }
    }
}
