#include "SysexFileDialog.h"
#include "SysexIO.h"
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include <sol/sol.hpp>

void openSaveSysexDialog(const std::string& partPrefix, I7Ed& ed)
{
    ed.saveSysex.partPrefix = partPrefix;
    IGFD::FileDialogConfig cfg;
    cfg.path              = ".";
    cfg.fileName          = "patch.syx";
    cfg.countSelectionMax = 1;
    cfg.flags             = ImGuiFileDialogFlags_ConfirmOverwrite;
    ImGuiFileDialog::Instance()->OpenDialog("SaveSysexDlg", "Save SysEx File", ".syx", cfg);
}

void openLoadSysexDialog()
{
    IGFD::FileDialogConfig cfg;
    cfg.path              = ".";
    cfg.countSelectionMax = 1;
    cfg.flags             = 0;
    ImGuiFileDialog::Instance()->OpenDialog("LoadSysexDlg", "Load SysEx File", ".syx", cfg);
}

void renderSysexFileDialogs(I7Ed& ed, SectionDef::NamedSections& sections)
{
    if (ImGuiFileDialog::Instance()->Display("LoadSysexDlg",
            ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            ed.loadSysex.filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            loadSysexFromFile(ed);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("SaveSysexDlg",
            ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            ed.saveSysex.filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            int n = partPrefixToNumber(ed.saveSysex.partPrefix);
            std::string msbId = "PRM-_PRF-_FP" + std::to_string(n) + "-NEFP_PAT_BS_MSB";
            SectionDef::FGetReceiveSysex msbGetter = [&ed, msbId]()
                -> std::vector<RequestMessage>
            {
                sol::function fn = ed.lua["CreateReceiveMessageForLeafId"];
                sol::object obj = fn(msbId);
                if (!obj.valid() || obj.get_type() == sol::type::lua_nil) { return {}; }
                return {obj.as<RequestMessage>()};
            };
            ed.saveSysex.phase = I7Ed::SaveSysexState::Phase::ReadMsb;
            triggerReceive(ed, {msbGetter});
        }
        ImGuiFileDialog::Instance()->Close();
    }
}
