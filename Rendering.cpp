#include "Rendering.h"
#include "SysexIO.h"
#include "imgui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"
#include "imgui_envelope.h"
#include "imgui_step_lfo.h"
#include "imsearch.h"
#include "ImGuiFileDialog.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <string>

// Persistent search text per param ID – survives across combo open/close cycles.
static std::unordered_map<std::string, std::string> gComboSearchText;

// Replacement for ImSearch::SearchBar() that restores the last query on open.
static void comboSearchBar(const std::string& paramId)
{
    auto& saved = gComboSearchText[paramId];

    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
        // Pre-populate ImSearch's query with the saved text so the filter is
        // active from the very first frame the combo is open.
        ImSearch::SetUserQuery(saved.c_str());
    }

    // Fixed-size buffer – queries are short, 512 chars is ample.
    char buf[512] = {};
    strncpy(buf, ImSearch::GetUserQuery(), sizeof(buf) - 1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputTextWithHint("##SearchBar", "Search", buf, sizeof(buf)))
    {
        ImSearch::SetUserQuery(buf);
    }
    // Persist the current query every frame so it survives combo close.
    saved = ImSearch::GetUserQuery();
}

void renderCombo(ParameterDef& param, I7Ed& ed)
{
    if (ImGui::BeginCombo(param.name().c_str(), param.stringValue.c_str()))
    {
        if (ImSearch::BeginSearch())
        {
            comboSearchBar(param.id);

            const ParameterDef::SelectionOptions resolvedOpts =
                param.optionsFn ? param.optionsFn() : param.options;
            for (const auto& [value, label] : resolvedOpts)
            {
                ImSearch::SearchableItem(label.c_str(),
                    [&](const char* name)
                    {
                        const bool isSelected = value == (int)param.value;
                        if (ImGui::Selectable(name, isSelected))
                        {
                            param.stringValue = name;
                            param.value = (float)value;
                            valueChanged(ed, param);
                        }
                    });
            }
            ImSearch::EndSearch();
        }
        ImGui::EndCombo();
    }
}

void drawReceiveButton(I7Ed& ed, const std::vector<SectionDef::FGetReceiveSysex>& getters)
{
    float btnW = ImGui::CalcTextSize("recv").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - btnW);
    if (ImGui::SmallButton("recv"))
    {
        triggerReceive(ed, getters);
    }
}

void renderTabbedSection(SectionDef& section, SectionDef::NamedSections& sections,
                         I7Ed& ed, ImVec2& canvasMax)
{
    if (!ImGui::Begin(section.name.c_str(), &section.isOpen))
    {
        ImGui::End();
        return;
    }
    {
        ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
        canvasMax.x = std::max(canvasMax.x, wp.x + ws.x);
        canvasMax.y = std::max(canvasMax.y, wp.y + ws.y);
    }

    std::vector<SectionDef::FGetReceiveSysex> getters;
    auto collectGetter = [&](const std::string& key)
    {
        auto it = sections.find(key);
        if (it != sections.end() && it->second.getReceiveSysex)
        {
            getters.push_back(it->second.getReceiveSysex);
        }
    };
    if (!section.tabCommonKey.empty())
    {
        collectGetter(section.tabCommonKey);
    }
    for (const auto& tab : section.tabs)
    {
        for (const auto& key : tab.sectionKeys)
        {
            collectGetter(key);
        }
    }
    if (!getters.empty())
    {
        drawReceiveButton(ed, getters);
    }

    if (!section.tabCommonKey.empty())
    {
        auto it = sections.find(section.tabCommonKey);
        if (it != sections.end())
        {
            renderSection(it->second, ed);
            ImGui::Separator();
        }
    }

    if (ImGui::BeginTabBar("##tabs"))
    {
        for (int ti = 0; ti < (int)section.tabs.size(); ++ti)
        {
            const auto& tab = section.tabs[ti];
            if (ImGui::BeginTabItem(tab.label.c_str()))
            {
                ImGui::PushID(ti);
                for (const auto& key : tab.sectionKeys)
                {
                    auto it = sections.find(key);
                    if (it != sections.end())
                    {
                        renderSection(it->second, ed);
                        for (auto& sub : it->second.subSections)
                        {
                            renderSection(sub, ed);
                        }
                    }
                }
                ImGui::PopID();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void renderSection(SectionDef& section, I7Ed& ed)
{
    constexpr float kRowSpacing = 15.0f;
    float lastKnobWidth = 0.0f;
    bool prevWasInline = false;
    bool isFirst = true;

    for (auto param : section.params)
    {
        if (param->name() == HIDDEN_PARAM_NAME)
        {
            continue;
        }
        bool isBlock = param->type == PARAM_TYPE_SELECTION
                    || param->type == PARAM_TYPE_TOGGLE
                    || param->type == PARAM_TYPE_ENVELOPE
                    || param->type == PARAM_TYPE_STEP_LFO;
        bool doSameLine = false;
        if (prevWasInline && !isBlock)
        {
            float nextX = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
            float rightEdge = ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x;
            doSameLine = (nextX + lastKnobWidth) <= rightEdge;
        }
        if (doSameLine)
        {
            ImGui::SameLine();
        }
        else if (!isFirst)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + kRowSpacing);
        }
        isFirst = false;

        if (param->type == PARAM_TYPE_RANGE)
        {
            if (ImGuiKnobs::Knob(param->name().c_str(), &param->value,
                    param->min(), param->max(), 0.0f, param->format.c_str(),
                    ImGuiKnobVariant_Tick, 0, ImGuiKnobFlags_AlwaysClamp))
            {
                valueChanged(ed, *param);
            }
            lastKnobWidth = ImGui::GetItemRectSize().x;
            prevWasInline = true;
        }
        else if (param->type == PARAM_TYPE_SELECTION)
        {
            renderCombo(*param, ed);
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_TOGGLE)
        {
            bool toggleVal = param->value != 0;
            if (ImGui::Toggle(param->name().c_str(), &toggleVal))
            {
                param->value = toggleVal ? 1.0f : 0.0f;
                valueChanged(ed, *param);
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_ENVELOPE)
        {
            const int nLevels = (int)param->levelIds.size();
            const int nTimes  = (int)param->timeIds.size();
            std::vector<float*> lvlPtrs(nLevels, nullptr);
            std::vector<float*> timPtrs(nTimes,  nullptr);
            for (int i = 0; i < nLevels; ++i)
            {
                auto* p = getParameterDef(ed, param->levelIds[i]);
                if (p) { lvlPtrs[i] = &p->value; }
            }
            for (int i = 0; i < nTimes; ++i)
            {
                auto* p = getParameterDef(ed, param->timeIds[i]);
                if (p) { timPtrs[i] = &p->value; }
            }
            bool allValid = true;
            for (auto* lp : lvlPtrs) { if (!lp) { allValid = false; break; } }
            for (auto* tp : timPtrs) { if (!tp) { allValid = false; break; } }

            if (allValid)
            {
                auto* firstLevel = getParameterDef(ed, param->levelIds[0]);
                auto* firstTime  = getParameterDef(ed, param->timeIds[0]);
                const float levelMin = firstLevel->min();
                const float levelMax = firstLevel->max();
                const float timeMax  = firstTime->max();

                std::vector<float> oldLvl(nLevels), oldTim(nTimes);
                for (int i = 0; i < nLevels; ++i) { oldLvl[i] = *lvlPtrs[i]; }
                for (int i = 0; i < nTimes;  ++i) { oldTim[i] = *timPtrs[i]; }

                if (ImEnvelope::EnvelopeWidget(param->id.c_str(),
                        lvlPtrs.data(), nLevels,
                        timPtrs.data(), nTimes,
                        levelMin, levelMax, timeMax,
                        ImVec2(0, 120.f), param->sustainSegment))
                {
                    for (int i = 0; i < nLevels; ++i)
                    {
                        if (*lvlPtrs[i] != oldLvl[i])
                        {
                            auto* p = getParameterDef(ed, param->levelIds[i]);
                            if (p) { valueChanged(ed, *p); }
                        }
                    }
                    for (int i = 0; i < nTimes; ++i)
                    {
                        if (*timPtrs[i] != oldTim[i])
                        {
                            auto* p = getParameterDef(ed, param->timeIds[i]);
                            if (p) { valueChanged(ed, *p); }
                        }
                    }
                }
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_STEP_LFO)
        {
            const int nSteps = (int)param->stepIds.size();
            std::vector<float*> stepPtrs(nSteps, nullptr);
            for (int i = 0; i < nSteps; ++i)
            {
                auto* p = getParameterDef(ed, param->stepIds[i]);
                if (p) { stepPtrs[i] = &p->value; }
            }
            bool allValid = true;
            for (auto* sp : stepPtrs) { if (!sp) { allValid = false; break; } }

            if (allValid)
            {
                auto* firstStep = getParameterDef(ed, param->stepIds[0]);
                const float valMin = firstStep->min();
                const float valMax = firstStep->max();

                float stepType = 0.f;
                auto* typeParam = getParameterDef(ed, param->stepTypeId);
                if (typeParam) { stepType = typeParam->value; }

                std::vector<float> oldVals(nSteps);
                for (int i = 0; i < nSteps; ++i) { oldVals[i] = *stepPtrs[i]; }

                if (ImStepLfo::StepLfoWidget(param->id.c_str(),
                        stepPtrs.data(), nSteps, stepType,
                        valMin, valMax, ImVec2(0, 80.f)))
                {
                    for (int i = 0; i < nSteps; ++i)
                    {
                        if (*stepPtrs[i] != oldVals[i])
                        {
                            auto* p = getParameterDef(ed, param->stepIds[i]);
                            if (p) { valueChanged(ed, *p); }
                        }
                    }
                }
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_ACTION)
        {
            if (ImGui::Button(param->name().c_str()))
            {
                triggerReceive(ed, {param->getAction});
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_SAVE_SYSEX)
        {
            if (ImGui::Button(param->name().c_str()))
            {
                ed.saveSysex.partPrefix = param->partPrefix;
                IGFD::FileDialogConfig cfg;
                cfg.path      = ".";
                cfg.fileName  = "patch.syx";
                cfg.countSelectionMax = 1;
                cfg.flags     = ImGuiFileDialogFlags_ConfirmOverwrite;
                ImGuiFileDialog::Instance()->OpenDialog(
                    "SaveSysexDlg", "Save SysEx File", ".syx", cfg);
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_LOAD_SYSEX)
        {
            if (ImGui::Button(param->name().c_str()))
            {
                IGFD::FileDialogConfig cfg;
                cfg.path      = ".";
                cfg.countSelectionMax = 1;
                cfg.flags     = 0;
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadSysexDlg", "Load SysEx File", ".syx", cfg);
            }
            prevWasInline = false;
        }
        else
        {
            std::cerr << "unknown param type: '" << param->type << "'" << std::endl;
            prevWasInline = false;
        }

        // Yellow highlight for the parameter found via "?" search
        if (!ed.highlightParamId.empty()
            && param->id == ed.highlightParamId
            && ed.highlightTimer > 0.f)
        {
            const float alpha = std::min(1.f, ed.highlightTimer);
            constexpr float pad = 4.f;
            const ImVec2 r0 = ImGui::GetItemRectMin();
            const ImVec2 r1 = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRect(
                {r0.x - pad, r0.y - pad}, {r1.x + pad, r1.y + pad},
                IM_COL32(255, 220, 0, (int)(alpha * 220)), 4.f, 0, 2.f);
        }
    }
}
