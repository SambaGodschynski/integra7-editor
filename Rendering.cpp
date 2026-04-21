#include "Rendering.h"
#include "SysexIO.h"
#include "imgui.h"
#include "imgui-knobs.h"
#include "imgui_knob_image.h"
#include "imgui_vslider_image.h"
#include "imgui_drawbar.h"
#include "imgui_toggle.h"
#include "imgui_envelope.h"
#include "imgui_step_lfo.h"
#include "imgui_range_slider.h"
#include "imsearch.h"
#include "ImGuiFileDialog.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <string>

// Default window size for views without persisted dimensions
constexpr float kDefaultWindowW = 640.0f;
constexpr float kDefaultWindowH = 480.0f;

// VSlider dimensions
constexpr float kVSliderTrackW     = 20.0f;
constexpr float kVSliderTrackH     = 80.0f;
constexpr float kVSliderHandleFactor = 0.25f;  // handle height as fraction of track height
constexpr float kVSliderHandleAspect = 1.0f;  // handle width = height * aspect

// Abbreviate a knob label word-by-word using common synth terminology.
// Short words are kept as-is; only words >= 5 chars with known abbreviations are shortened.
// The full name is still shown as a tooltip when the label differs from the original.
static std::string abbrevLabel(const std::string& name)
{
    static const std::pair<const char*, const char*> kAbbrev[] = {
        { "Portamento",  "Port"  },
        { "Sensitivity", "Sens"  },
        { "Modulation",  "Mod"   },
        { "Percussion",  "Perc"  },
        { "Variation",   "Var"   },
        { "Resonance",   "Res"   },
        { "Frequency",   "Frq"   },
        { "Recharge",    "Rchg"  },
        { "Velocity",    "Vel"   },
        { "Envelope",    "Env"   },
        { "Harmonic",    "Hrm"   },
        { "Keyboard",    "Key"   },
        { "Interval",    "Intv"  },
        { "Pressure",    "Prs"   },
        { "Leakage",     "Lkg"   },
        { "Vibrato",     "Vib"   },
        { "Control",     "Ctrl"  },
        { "Release",     "Rel"   },
        { "Sustain",     "Sus"   },
        { "Reverb",      "Rev"   },
        { "Filter",      "Flt"   },
        { "Chorus",      "Cho"   },
        { "Volume",      "Vol"   },
        { "Offset",      "Ofs"   },
        { "Cutoff",      "Cut"   },
        { "Detune",      "Det"   },
        { "Attack",      "Atk"   },
        { "Normal",      "Nrm"   },
        { "Assign",      "Asgn"  },
        { "Output",      "Out"   },
        { "Source",      "Src"   },
        { "Number",      "Nr"    },
        { "Switch",      "Sw"    },
        { "Follow",      "Flw"   },
        { "Random",      "Rnd"   },
        { "Legato",      "Leg"   },
        { "Coarse",      "Crs"   },
        { "Boost",       "Bst"   },
        { "Width",       "Wdth"  },
        { "Shift",       "Shft"  },
        { "Shape",       "Shp"   },
        { "Scale",       "Scl"   },
        { "Range",       "Rng"   },
        { "Noise",       "Nse"   },
        { "Click",       "Clk"   },
        { "Pitch",       "Pit"   },
        { "Decay",       "Dcy"   },
        { "Depth",       "Dpt"   },
        { "Delay",       "Dly"   },
        { "Level",       "Lvl"   },
        { "Phase",       "Ph"    },
        { "Input",       "In"    },
    };

    std::string result;
    std::string word;

    auto flushWord = [&]()
    {
        if (word.empty()) { return; }
        for (auto& [from, to] : kAbbrev)
        {
            if (word == from)
            {
                if (!result.empty()) { result += ' '; }
                result += to;
                word.clear();
                return;
            }
        }
        if (!result.empty()) { result += ' '; }
        result += word;
        word.clear();
    };

    for (char c : name)
    {
        if (c == ' ') { flushWord(); }
        else          { word += c;   }
    }
    flushWord();
    return result;
}

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
    std::string comboLabel = param.name() + "##" + param.id;
    if (ImGui::BeginCombo(comboLabel.c_str(), param.stringValue.c_str()))
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
    ImGui::SetNextWindowSize(ImVec2(kDefaultWindowW, kDefaultWindowH), ImGuiCond_FirstUseEver);
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
        for (const auto& ref : tab.sectionKeys)
        {
            collectGetter(ref.key);
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

    const bool isNavigateTarget = (section.name == ed.navigateOpenerName
                                   && !ed.navigateTabLabel.empty());

    if (ImGui::BeginTabBar("##tabs"))
    {
        for (int ti = 0; ti < (int)section.tabs.size(); ++ti)
        {
            const auto& tab = section.tabs[ti];
            ImGuiTabItemFlags tabFlags = 0;
            if (isNavigateTarget && tab.label == ed.navigateTabLabel)
            {
                tabFlags = ImGuiTabItemFlags_SetSelected;
            }
            if (ImGui::BeginTabItem(tab.label.c_str(), nullptr, tabFlags))
            {
                ImGui::PushID(ti);
                for (const auto& ref : tab.sectionKeys)
                {
                    auto it = sections.find(ref.key);
                    if (it != sections.end())
                    {
                        bool visible = true;
                        if (!ref.accordionLabel.empty())
                        {
                            if (isNavigateTarget
                                && ref.accordionLabel == ed.navigateAccordionLabel)
                            {
                                ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                            }
                            visible = ImGui::CollapsingHeader(ref.accordionLabel.c_str());
                        }
                        if (visible)
                        {
                            renderSection(it->second, ed);
                            for (auto& sub : it->second.subSections)
                            {
                                renderSection(sub, ed);
                            }
                        }
                    }
                }
                ImGui::PopID();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    if (isNavigateTarget)
    {
        ed.navigateOpenerName.clear();
        ed.navigateTabLabel.clear();
        ed.navigateAccordionLabel.clear();
    }
    ImGui::End();
}

void renderSection(SectionDef& section, I7Ed& ed)
{
    constexpr float kRowSpacing = 15.0f;
    float lastKnobWidth = 0.0f;
    bool prevWasInline  = false;  // last item was a knob / inline widget
    bool prevWasToggle  = false;  // last item was a toggle
    bool isFirst = true;

    for (auto param : section.params)
    {
        if (param->type == PARAM_TYPE_NEWLINE)
        {
            if (prevWasInline) { ImGui::NewLine(); }
            prevWasInline = false;
            prevWasToggle = false;
            isFirst = true;
            continue;
        }
        if (param->type == PARAM_TYPE_SEPARATOR)
        {
            if (prevWasInline) { ImGui::NewLine(); }
            ImGui::SeparatorText(param->name().c_str());
            prevWasInline = false;
            prevWasToggle = false;
            isFirst = true;
            continue;
        }
        if (param->name() == HIDDEN_PARAM_NAME)
        {
            continue;
        }
        bool inlineToggles = (section.layout == "inline_toggles");
        bool isInlineSelect = param->type == PARAM_TYPE_SELECTION && param->size > 0.0f;
        bool isBinaryRange  = (param->type == PARAM_TYPE_RANGE
                               && param->min() == 0.0f && param->max() == 1.0f);
        bool isToggleType   = (param->type == PARAM_TYPE_TOGGLE) || isBinaryRange;
        bool isBlock = (param->type == PARAM_TYPE_SELECTION && !isInlineSelect)
                    || (!inlineToggles && isToggleType)
                    || param->type == PARAM_TYPE_SOLO_TOGGLE
                    || param->type == PARAM_TYPE_ENVELOPE
                    || param->type == PARAM_TYPE_STEP_LFO;

        bool doSameLine = false;
        if (prevWasInline && !isBlock)
        {
            // knob → knob: inline if there is room
            float nextX    = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
            float rightEdge = ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x;
            doSameLine = (nextX + lastKnobWidth) <= rightEdge;
        }
        else if (prevWasToggle && isToggleType && !inlineToggles)
        {
            // toggle → toggle: inline if there is room
            float nextX    = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
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
            if (param->valueOverride)
            {
                param->value = param->valueOverride();
            }
            if (isBinaryRange)
            {
                // Binary [0,1] range: render as toggle, grouped with other toggles
                bool on = (param->value != 0.0f);
                if (ImGui::Toggle(param->name().c_str(), &on))
                {
                    param->value = on ? 1.0f : 0.0f;
                    valueChanged(ed, *param);
                }
                lastKnobWidth = ImGui::GetItemRectSize().x;
                prevWasInline = false;
                prevWasToggle = true;
            }
            else
            {
                ImGuiKnobFlags knobFlags = ImGuiKnobFlags_AlwaysClamp;
                if (param->noTitle) { knobFlags |= ImGuiKnobFlags_NoTitle; }
                if (param->noInput) { knobFlags |= ImGuiKnobFlags_NoInput; }
                std::string fullName   = param->name();
                std::string dispName   = abbrevLabel(fullName);
                bool        abbreviated = (dispName != fullName);
                ImGui::PushID(param->id.c_str());
                if (ImKnobImage::Knob(dispName.c_str(), &param->value,
                        param->min(), param->max(), 0.0f, param->format.c_str(),
                        ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, param->size, knobFlags))
                {
                    valueChanged(ed, *param);
                }
                ImGui::PopID();
                if (ImGui::IsItemHovered() && (param->noTitle || abbreviated))
                {
                    ImGui::SetTooltip("%s: %.0f", fullName.c_str(), param->value);
                }
                lastKnobWidth = ImGui::GetItemRectSize().x;
                prevWasInline = true;
                prevWasToggle = false;
            }
        }
        else if (param->type == PARAM_TYPE_VSLIDER)
        {
            std::string vsLabel = "##" + param->id;
            if (ImVSliderImage::VSlider(vsLabel.c_str(), ImVec2(kVSliderTrackW, kVSliderTrackH), &param->value,
                    param->min(), param->max(), ed.sliderHandleTex,
                    kVSliderTrackH * kVSliderHandleFactor * kVSliderHandleAspect,
                    kVSliderTrackH * kVSliderHandleFactor))
            {
                valueChanged(ed, *param);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s: %.0f", param->name().c_str(), param->value);
            }
            lastKnobWidth = 20.0f;
            prevWasInline = true;
            prevWasToggle = false;
        }
        else if (param->type == PARAM_TYPE_SELECTION)
        {
            if (isInlineSelect)
            {
                ImGui::SetNextItemWidth(param->size);
            }
            renderCombo(*param, ed);
            if (isInlineSelect)
            {
                lastKnobWidth = param->size;
                prevWasInline = true;
                prevWasToggle = false;
            }
            else
            {
                prevWasInline = false;
                prevWasToggle = false;
            }
        }
        else if (param->type == PARAM_TYPE_TOGGLE)
        {
            bool toggleVal = param->value != 0;
            if (ImGui::Toggle(param->name().c_str(), &toggleVal))
            {
                param->value = toggleVal ? 1.0f : 0.0f;
                valueChanged(ed, *param);
            }
            lastKnobWidth = ImGui::GetItemRectSize().x;
            if (inlineToggles)
            {
                prevWasInline = true;
                prevWasToggle = false;
            }
            else
            {
                prevWasInline = false;
                prevWasToggle = true;
            }
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
            if (ImGui::Button((param->name() + "##" + param->id).c_str()))
            {
                triggerReceive(ed, {param->getAction});
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_INPUTTEXT)
        {
            char buf[17] = {};
            strncpy(buf, param->stringValue.c_str(), 16);
            float w = ImGui::CalcTextSize("AAAAAAAAAAAAAAAA").x
                    + ImGui::GetStyle().FramePadding.x * 2.0f;
            ImGui::SetNextItemWidth(w);
            ImGui::InputText(("##it_" + param->id).c_str(), buf, sizeof(buf));
            bool active = ImGui::IsItemActive();
            if (active)
            {
                param->stringValue = buf;
            }
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                param->stringValue = buf;
                if (param->setStringValue)
                {
                    auto sysex = param->setStringValue(param->stringValue);
                    sendMessage(ed, sysex);
                }
            }
            if (!active && param->stringValueGetter)
            {
                param->stringValue = param->stringValueGetter();
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(param->name().c_str());
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_SAVE_SYSEX)
        {
            if (ImGui::Button((param->name() + "##" + param->id).c_str()))
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
            if (ImGui::Button((param->name() + "##" + param->id).c_str()))
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
        else if (param->type == PARAM_TYPE_SOLO_TOGGLE)
        {
            auto* linked = getParameterDef(ed, param->linkedParamId);
            bool isSoloed = linked && (linked->value == param->linkedValue);
            if (ImGui::Toggle(param->name().c_str(), &isSoloed))
            {
                if (linked)
                {
                    linked->value = isSoloed ? param->linkedValue : 0.0f;
                    valueChanged(ed, *linked);
                }
            }
            prevWasInline = false;
        }
        else if (param->type != PARAM_TYPE_NEWLINE)
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

void renderEq3Band(SectionDef& section, I7Ed& ed)
{
    // Expected param order: SW, LowGain, MidGain, HighGain, LowFreq, MidFreq, MidQ, HighFreq
    std::vector<ParameterDef*> ps(section.params.begin(), section.params.end());
    if (ps.size() < 8)
    {
        renderSection(section, ed);
        return;
    }

    auto* swParam  = ps[0];
    auto* lowGain  = ps[1];
    auto* midGain  = ps[2];
    auto* highGain = ps[3];
    auto* lowFreq  = ps[4];
    auto* midFreq  = ps[5];
    auto* midQ     = ps[6];
    auto* highFreq = ps[7];

    // EQ on/off toggle
    bool sw = swParam->value != 0;
    if (ImGui::Toggle(swParam->name().c_str(), &sw))
    {
        swParam->value = sw ? 1.0f : 0.0f;
        valueChanged(ed, *swParam);
    }

    ImGui::Spacing();

    constexpr ImGuiKnobFlags kKnobFlags =
        ImGuiKnobFlags_AlwaysClamp | ImGuiKnobFlags_NoTitle | ImGuiKnobFlags_NoInput;
    constexpr float kKnobSize  = 25.0f;
    constexpr float kColGap    = 20.0f;

    auto renderVS = [&](ParameterDef* p)
    {
        std::string lbl = "##" + p->id;
        if (ImVSliderImage::VSlider(lbl.c_str(), ImVec2(kVSliderTrackW, kVSliderTrackH), &p->value,
                p->min(), p->max(), ed.sliderHandleTex,
                kVSliderTrackH * kVSliderHandleFactor * kVSliderHandleAspect,
                kVSliderTrackH * kVSliderHandleFactor))
        {
            valueChanged(ed, *p);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s: %.0f", p->name().c_str(), p->value);
        }
    };

    auto renderKnob = [&](ParameterDef* p)
    {
        if (ImKnobImage::Knob(p->name().c_str(), &p->value,
                p->min(), p->max(), 0.0f, "%.0f",
                ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, kKnobSize, kKnobFlags))
        {
            valueChanged(ed, *p);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s: %.0f", p->name().c_str(), p->value);
        }
    };

    // LOW band
    ImGui::BeginGroup();
    ImGui::TextUnformatted("LOW");
    renderVS(lowGain);
    renderKnob(lowFreq);
    ImGui::EndGroup();

    ImGui::SameLine(0, kColGap);

    // MID band
    ImGui::BeginGroup();
    ImGui::TextUnformatted("MID");
    renderVS(midGain);
    renderKnob(midFreq);
    ImGui::SameLine();
    renderKnob(midQ);
    ImGui::EndGroup();

    ImGui::SameLine(0, kColGap);

    // HIGH band
    ImGui::BeginGroup();
    ImGui::TextUnformatted("HIGH");
    renderVS(highGain);
    renderKnob(highFreq);
    ImGui::EndGroup();
}

void renderKeyboard(SectionDef& section, I7Ed& ed)
{
    // Expected param order (10 params):
    //  0: KFADE_LO   1: KRANGE_LO  2: KRANGE_UP  3: KFADE_UP
    //  4: VFADE_LO   5: VRANGE_LO  6: VRANGE_UP  7: VFADE_UP
    //  8: VSENS_OFST 9: VELO_CRV_TYPE
    std::vector<ParameterDef*> ps(section.params.begin(), section.params.end());
    if (ps.size() < 10)
    {
        renderSection(section, ed);
        return;
    }

    auto* kFadeLo   = ps[0];
    auto* kRangeLo  = ps[1];
    auto* kRangeHi  = ps[2];
    auto* kFadeHi   = ps[3];
    auto* vFadeLo   = ps[4];
    auto* vRangeLo  = ps[5];
    auto* vRangeHi  = ps[6];
    auto* vFadeHi   = ps[7];
    auto* vSensOfst = ps[8];
    auto* veloCrv   = ps[9];

    // Standard knob style (same as renderSection: title + input, default size)
    constexpr ImGuiKnobFlags kKnobFlags = ImGuiKnobFlags_AlwaysClamp;

    auto renderKnob = [&](ParameterDef* p)
    {
        if (ImKnobImage::Knob(p->name().c_str(), &p->value,
                p->min(), p->max(), 0.0f, p->format.c_str(),
                ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, p->size, kKnobFlags))
        {
            valueChanged(ed, *p);
        }
    };

    // Range slider helper: renders label + full-width slider on its own line
    auto renderRange = [&](ParameterDef* pLo, ParameterDef* pHi, const char* lbl)
    {
        ImGui::TextUnformatted(lbl);
        std::string wid = "##range_" + pLo->id;
        int lo = (int)pLo->value;
        int hi = (int)pHi->value;
        ImGui::SetNextItemWidth(-FLT_MIN); // full remaining width
        if (RangeSliderInt(wid.c_str(), &lo, &hi, (int)pLo->min(), (int)pHi->max()))
        {
            if (lo != (int)pLo->value)
            {
                pLo->value = (float)lo;
                valueChanged(ed, *pLo);
            }
            if (hi != (int)pHi->value)
            {
                pHi->value = (float)hi;
                valueChanged(ed, *pHi);
            }
        }
    };

    // ---- KEYBOARD ZONE ----
    ImGui::SeparatorText("KEYBOARD");
    ImGui::Spacing();

    renderKnob(kFadeLo);
    ImGui::SameLine();
    renderKnob(kFadeHi);

    ImGui::Spacing();
    renderRange(kRangeLo, kRangeHi, "Key Range Lo/Hi");

    ImGui::Spacing();
    ImGui::SeparatorText("VELOCITY");
    ImGui::Spacing();

    renderKnob(vFadeLo);
    ImGui::SameLine();
    renderKnob(vFadeHi);
    ImGui::SameLine();
    renderKnob(vSensOfst);

    ImGui::Spacing();
    renderRange(vRangeLo, vRangeHi, "Vel Range Lo/Hi");

    ImGui::Spacing();
    renderCombo(*veloCrv, ed);
}

void renderRssXY(SectionDef& section, I7Ed& ed)
{
    // Expected: 64 params in order X,Y,Width,RevSend × 16 parts
    std::vector<ParameterDef*> ps(section.params.begin(), section.params.end());
    if ((int)ps.size() < 64) { renderSection(section, ed); return; }

    // Part colours – one distinct colour per part
    static const ImU32 kPartColors[16] = {
        IM_COL32(220, 80,  80,  255), IM_COL32( 80, 180,  80, 255),
        IM_COL32( 80, 130, 220, 255), IM_COL32(220, 200,  60, 255),
        IM_COL32(200,  80, 200, 255), IM_COL32( 60, 200, 200, 255),
        IM_COL32(230, 140,  50, 255), IM_COL32(140, 220, 100, 255),
        IM_COL32(100, 160, 240, 255), IM_COL32(240, 120, 160, 255),
        IM_COL32(160, 240, 180, 255), IM_COL32(240, 200, 100, 255),
        IM_COL32(180, 100, 240, 255), IM_COL32(100, 220, 240, 255),
        IM_COL32(240, 160,  80, 255), IM_COL32(180, 180, 180, 255),
    };

    constexpr float kDotR    = 10.0f;
    constexpr float kMinSize = 220.0f;
    constexpr float kMaxSize = 480.0f;

    float avail     = ImGui::GetContentRegionAvail().x;
    float canvasSize = std::clamp(avail, kMinSize, kMaxSize);

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##rss_canvas", ImVec2(canvasSize, canvasSize));
    bool canvasActive = ImGui::IsItemHovered() || ImGui::IsItemActive();

    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2 cMax     = { origin.x + canvasSize, origin.y + canvasSize };
    float  cx       = origin.x + canvasSize * 0.5f;
    float  cy       = origin.y + canvasSize * 0.5f;

    // Background
    dl->AddRectFilled(origin, cMax, IM_COL32(25, 25, 35, 255), 4.0f);
    dl->AddRect      (origin, cMax, IM_COL32(80, 80, 100, 200), 4.0f);

    // Surround field circles
    for (int ring = 1; ring <= 3; ++ring)
    {
        float r = canvasSize * 0.5f * ring / 3.0f;
        dl->AddCircle(ImVec2(cx, cy), r, IM_COL32(60, 60, 80, 180));
    }
    // Cross-hair
    dl->AddLine({cx, origin.y + 4}, {cx, cMax.y - 4}, IM_COL32(60, 60, 80, 180));
    dl->AddLine({origin.x + 4, cy}, {cMax.x - 4, cy}, IM_COL32(60, 60, 80, 180));

    // Labels: FRONT / BACK / L / R
    ImFont* font = ImGui::GetFont();
    float   fs   = ImGui::GetFontSize() * 0.8f;
    dl->AddText(font, fs, {cx - 16, origin.y + 4},       IM_COL32(120,120,140,200), "FRONT");
    dl->AddText(font, fs, {cx - 14, cMax.y  - fs - 4},   IM_COL32(120,120,140,200), "BACK");
    dl->AddText(font, fs, {origin.x + 4, cy - fs * 0.5f},IM_COL32(120,120,140,200), "L");
    dl->AddText(font, fs, {cMax.x - 10,  cy - fs * 0.5f},IM_COL32(120,120,140,200), "R");

    // Listener symbol at canvas centre
    dl->AddCircle    (ImVec2(cx, cy), 6.0f, IM_COL32(220, 220, 220, 220), 12, 1.5f);
    dl->AddCircleFilled(ImVec2(cx, cy), 3.0f, IM_COL32(220, 220, 220, 220));

    // Drag state (one instance is fine – only one RSS view exists)
    static int  sDragging = -1;
    ImVec2 mouse = ImGui::GetIO().MousePos;
    bool   lDown = ImGui::GetIO().MouseDown[0];
    bool   lClick= ImGui::GetIO().MouseClicked[0];
    if (!lDown) sDragging = -1;

    // Tooltip text built up while iterating
    int    hoveredPart = -1;

    for (int i = 0; i < 16; ++i)
    {
        ParameterDef* px = ps[i * 4 + 0];
        ParameterDef* py = ps[i * 4 + 1];

        float nx = px->value / 127.0f;
        float ny = py->value / 127.0f;
        float dotX = origin.x + nx * canvasSize;
        float dotY = origin.y + ny * canvasSize;

        // Hit-test
        float ddx = mouse.x - dotX, ddy = mouse.y - dotY;
        bool  hit = (ddx * ddx + ddy * ddy) <= kDotR * kDotR;

        if (hit && lClick && canvasActive && sDragging == -1)
            sDragging = i;

        if (sDragging == i && lDown)
        {
            float newX = std::clamp((mouse.x - origin.x) / canvasSize * 127.0f, 0.0f, 127.0f);
            float newY = std::clamp((mouse.y - origin.y) / canvasSize * 127.0f, 0.0f, 127.0f);
            if ((int)newX != (int)px->value) { px->value = newX; valueChanged(ed, *px); }
            if ((int)newY != (int)py->value) { py->value = newY; valueChanged(ed, *py); }
            // Recalc dot position after update
            dotX = origin.x + (px->value / 127.0f) * canvasSize;
            dotY = origin.y + (py->value / 127.0f) * canvasSize;
        }

        if (hit || sDragging == i) hoveredPart = i;

        // Draw dot
        ImU32 col  = kPartColors[i];
        float rDot = (sDragging == i) ? kDotR + 2.0f : kDotR;
        dl->AddCircleFilled(ImVec2(dotX, dotY), rDot, col);
        dl->AddCircle      (ImVec2(dotX, dotY), rDot, IM_COL32(255, 255, 255, 120));

        // Part number label
        char lbl[4];
        snprintf(lbl, sizeof(lbl), "%d", i + 1);
        ImVec2 ts = ImGui::CalcTextSize(lbl);
        dl->AddText(ImVec2(dotX - ts.x * 0.5f, dotY - ts.y * 0.5f),
                    IM_COL32(255, 255, 255, 230), lbl);
    }

    // Tooltip for hovered / dragged part
    if (hoveredPart >= 0)
    {
        ParameterDef* px = ps[hoveredPart * 4 + 0];
        ParameterDef* py = ps[hoveredPart * 4 + 1];
        ParameterDef* pw = ps[hoveredPart * 4 + 2];
        ParameterDef* pr = ps[hoveredPart * 4 + 3];
        ImGui::BeginTooltip();
        ImGui::Text("Part %d", hoveredPart + 1);
        ImGui::Text("X: %d  Y: %d", (int)px->value, (int)py->value);
        ImGui::Text("Width: %d  RevSend: %d", (int)pw->value, (int)pr->value);
        ImGui::EndTooltip();
    }

    // Width and Ambience Send knobs for all 16 parts
    constexpr float kKnobSize = 42.0f;
    constexpr ImGuiKnobFlags kKF = ImGuiKnobFlags_AlwaysClamp;

    auto renderWrappingKnobs = [&](int paramOffset, const char* label)
    {
        ImGui::Spacing();
        ImGui::SeparatorText(label);
        ImGui::Spacing();
        for (int i = 0; i < 16; ++i)
        {
            ParameterDef* pk = ps[i * 4 + paramOffset];
            if (i > 0)
            {
                float nextX    = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
                float rightEdge = ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x;
                if (nextX + kKnobSize <= rightEdge)
                    ImGui::SameLine();
            }
            if (ImKnobImage::Knob(pk->name().c_str(), &pk->value,
                    pk->min(), pk->max(), 0.0f, "%.0f",
                    ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, kKnobSize, kKF))
            {
                valueChanged(ed, *pk);
            }
        }
    };

    renderWrappingKnobs(2, "Part Width");
    renderWrappingKnobs(3, "Part Ambience Send");
}

void renderMixer(SectionDef& /*section*/, I7Ed& ed)
{
    constexpr float kStripW  = 68.0f;
    constexpr float kSliderW = 20.0f;
    constexpr float kSliderH = 150.0f;
    constexpr float kKnobPan  = 44.0f;
    constexpr float kKnobSend = 32.0f;
    constexpr int   kParts    = 16;

    const std::string soloId = "PRM-_PRF-_FC-NEFC_SOLO_PART";
    auto* soloParam = getParameterDef(ed, soloId);

    // helper: push active-button color when condition holds
    auto pushActive = [](bool active)
    {
        if (active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }
        return active;
    };

    ImGui::BeginChild("##mixer", ImVec2(0.0f, 0.0f), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    for (int n = 1; n <= kParts; ++n)
    {
        std::string fp = "PRM-_PRF-_FP" + std::to_string(n) + "-NEFP_";
        auto* pLevel = getParameterDef(ed, fp + "LEVEL");
        auto* pPan   = getParameterDef(ed, fp + "PAN");
        auto* pCho   = getParameterDef(ed, fp + "CHO_SEND");
        auto* pRev   = getParameterDef(ed, fp + "REV_SEND");
        auto* pMute  = getParameterDef(ed, fp + "MUTE_SW");

        if (n > 1) { ImGui::SameLine(0.0f, 12.0f); }

        ImGui::PushID(n);
        ImGui::BeginGroup();

        float groupX = ImGui::GetCursorPosX();

        // ── Part label ─────────────────────────────────────────────────────
        char label[8];
        snprintf(label, sizeof(label), "%02d", n);
        float labelW = ImGui::CalcTextSize(label).x;
        ImGui::SetCursorPosX(groupX + (kStripW - labelW) * 0.5f);
        ImGui::TextUnformatted(label);

        // ── Volume VSlider ─────────────────────────────────────────────────
        ImGui::SetCursorPosX(groupX + (kStripW - kSliderW) * 0.5f);
        if (pLevel)
        {
            std::string slId = "##vol" + std::to_string(n);
            if (ImVSliderImage::VSlider(slId.c_str(), ImVec2(kSliderW, kSliderH),
                    &pLevel->value, pLevel->min(), pLevel->max(),
                    ed.sliderHandleTex,
                    kSliderH * kVSliderHandleFactor * kVSliderHandleAspect,
                    kSliderH * kVSliderHandleFactor))
            {
                valueChanged(ed, *pLevel);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Vol: %.0f", pLevel->value);
            }
        }

        // ── Pan knob ───────────────────────────────────────────────────────
        ImGui::SetCursorPosX(groupX + (kStripW - kKnobPan) * 0.5f);
        if (pPan)
        {
            if (ImKnobImage::Knob("##pan", &pPan->value,
                    pPan->min(), pPan->max(), 0.0f, "%+.0f",
                    ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, kKnobPan,
                    ImGuiKnobFlags_NoTitle))
            {
                valueChanged(ed, *pPan);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Pan: %+.0f", pPan->value);
            }
        }

        // ── Rev + Cho small knobs side by side ─────────────────────────────
        float sendsW = kKnobSend * 2.0f + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(groupX + (kStripW - sendsW) * 0.5f);
        if (pRev)
        {
            if (ImKnobImage::Knob("##rev", &pRev->value,
                    pRev->min(), pRev->max(), 0.0f, "%.0f",
                    ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, kKnobSend,
                    ImGuiKnobFlags_NoTitle))
            {
                valueChanged(ed, *pRev);
            }
            if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Rev: %.0f", pRev->value); }
        }
        ImGui::SameLine();
        if (pCho)
        {
            if (ImKnobImage::Knob("##cho", &pCho->value,
                    pCho->min(), pCho->max(), 0.0f, "%.0f",
                    ed.knobTexture, ed.knobAtlasFrames, ed.knobAtlasCols, kKnobSend,
                    ImGuiKnobFlags_NoTitle))
            {
                valueChanged(ed, *pCho);
            }
            if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Cho: %.0f", pCho->value); }
        }

        // ── Mute / Solo buttons ────────────────────────────────────────────
        float btnW  = (kStripW - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        ImGui::SetCursorPosX(groupX);

        bool isMuted = pMute && pMute->value != 0.0f;
        bool pushed  = pushActive(isMuted);
        if (ImGui::Button("M##m", ImVec2(btnW, 0.0f)))
        {
            if (pMute)
            {
                pMute->value = isMuted ? 0.0f : 1.0f;
                valueChanged(ed, *pMute);
            }
        }
        if (pushed) { ImGui::PopStyleColor(); }
        if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Mute"); }

        ImGui::SameLine();

        bool isSoloed = soloParam && soloParam->value == (float)n;
        pushed = pushActive(isSoloed);
        if (ImGui::Button("S##s", ImVec2(btnW, 0.0f)))
        {
            if (soloParam)
            {
                soloParam->value = isSoloed ? 0.0f : (float)n;
                valueChanged(ed, *soloParam);
            }
        }
        if (pushed) { ImGui::PopStyleColor(); }
        if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Solo"); }

        ImGui::EndGroup();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void renderDrawbars(SectionDef& section, I7Ed& ed)
{
    constexpr float kImgW          = 38.0f;
    constexpr float kSlotH         = 220.0f;   // >= img_disp_h (232px) so full shaft is reachable
    constexpr float kImgNativeH    = 256.0f;
    constexpr float kHandleNativeH = 45.0f;   // handle height at bottom of image (native px)
    constexpr float kGap           =  6.0f;

    // Fallback to normal rendering when no drawbar params are active (non-TW-Organ instrument)
    bool hasDrawbars = false;
    for (auto* p : section.params)
    {
        if (p->drawbarColor && !p->drawbarColor().empty())
        {
            hasDrawbars = true;
            break;
        }
    }
    if (!hasDrawbars)
    {
        renderSection(section, ed);
        return;
    }

    // Render select params (Inst. Number) above the drawbars
    for (auto* param : section.params)
    {
        if (param->drawbarColor) { continue; }
        if (param->name() == HIDDEN_PARAM_NAME) { continue; }
        if (param->type == PARAM_TYPE_SELECTION)
        {
            renderCombo(*param, ed);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
        }
    }

    // Drawbar row
    bool firstDb = true;
    for (auto* param : section.params)
    {
        if (!param->drawbarColor) { continue; }

        std::string color = param->drawbarColor();
        if (color.empty()) { continue; }

        if (!firstDb) { ImGui::SameLine(0.0f, kGap); }
        firstDb = false;

        if (param->valueOverride) { param->value = param->valueOverride(); }

        ImTextureID tex = 0;
        if      (color == "bk") { tex = ed.drawbarTexBk; }
        else if (color == "wt") { tex = ed.drawbarTexWt; }
        else if (color == "br") { tex = ed.drawbarTexBr; }

        // Extract short pitch label from "Harmonic Bar X", prettify fractions
        std::string fullName = param->name();
        const std::string pfx = "Harmonic Bar ";
        std::string shortName = (fullName.rfind(pfx, 0) == 0)
            ? fullName.substr(pfx.size())
            : fullName;
        // Replace "-" separator before fractions with a space: "5-1/3" -> "5 1/3"
        {
            auto pos = shortName.find('-');
            if (pos != std::string::npos) { shortName[pos] = ' '; }
        }

        ImGui::BeginGroup();

        std::string lbl = "##db_" + param->id;
        if (ImDrawbar::Drawbar(lbl.c_str(), &param->value,
                param->min(), param->max(),
                tex, kImgW, kSlotH, kImgNativeH, kHandleNativeH))
        {
            valueChanged(ed, *param);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s: %.0f", fullName.c_str(), param->value);
        }

        // Centered pitch label below the drawbar
        float labelW = ImGui::CalcTextSize(shortName.c_str()).x;
        float indent = (kImgW - labelW) * 0.5f;
        if (indent > 0.0f) { ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent); }
        ImGui::TextUnformatted(shortName.c_str());

        ImGui::EndGroup();
    }
    ImGui::NewLine();

    // Render remaining non-drawbar range params below
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    SectionDef remainder;
    for (auto* param : section.params)
    {
        if (param->drawbarColor) { continue; }
        if (param->type == PARAM_TYPE_SELECTION) { continue; }
        remainder.params.push_back(param);
    }
    if (!remainder.params.empty())
    {
        renderSection(remainder, ed);
    }
}
