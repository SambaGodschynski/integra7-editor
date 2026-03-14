require "math"
require "_sysex"
require "_com"
require "_model"
require "_waveData"
require "_srxWaveData"

local get = GetWrapper

local function i7offset(offset) return function(gui) return math.tointeger(offset + gui) end end
local function guiOffset(offset) return function(i7)  return math.tointeger(i7 - offset)  end end

local NoteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}

local function noteName(note)
    local octave = math.floor(note / 12) - 1
    return NoteNames[(note % 12) + 1] .. tostring(octave)
end

local NoteOptions = {}
for note = 21, 88, 1 do
    NoteOptions[note - 21] = noteName(note)   -- [0]="A0" .. [67]="E6"
end

local AssignType  = {[0]="Multi",   [1]="Single"}
local EnvMode     = {[0]="NO-SUS",  [1]="SUSTAIN"}
local OutAssign   = {
    [0]="PART",      [1]="COMP+EQ 1", [2]="COMP+EQ 2",
    [3]="COMP+EQ 3", [4]="COMP+EQ 4", [5]="COMP+EQ 5", [6]="COMP+EQ 6",
}
local WmtVeloCtrl = {[0]="OFF", [1]="ON", [2]="RANDOM"}

-- per-part/wmt GID state for dynamic wave options
local currentGidPcmd = {}

local function waveOptionsForPcmd(partNr, wmtNr)
    local gid = ((currentGidPcmd[partNr] or {})[wmtNr]) or 0
    if gid >= 1 and gid <= 12 and SrxWaveNames[gid] then
        return SrxWaveNames[gid]
    end
    return WaveNames
end
local WaveGroup   = {
    [0]="Internal (XV-5080)",
    [1]="SRX-01", [2]="SRX-02", [3]="SRX-03", [4]="SRX-04",
    [5]="SRX-05", [6]="SRX-06", [7]="SRX-07", [8]="SRX-08",
    [9]="SRX-09", [10]="SRX-10", [11]="SRX-11", [12]="SRX-12",
}
local WaveGain    = {[0]="-6dB", [1]="0dB", [2]="+6dB", [3]="+12dB"}
local FxmColor    = {[0]="1", [1]="2", [2]="3", [3]="4"}
local AltPanSw    = {[0]="OFF", [1]="ON", [2]="REVERSE"}
local FilterType  = {[0]="OFF", [1]="LPF", [2]="BPF", [3]="HPF", [4]="PKG", [5]="LPF2", [6]="LPF3"}
local VCurve      = {[0]="FIXED", [1]="1", [2]="2", [3]="3", [4]="4", [5]="5", [6]="6", [7]="7"}

-- Mapping from RFRT node IDs to fake param ID suffixes
local nodeToFakeSuffix = {
    -- Partial Common
    RFRT_ASGN_TYPE     = "_PcmdAsgnType",
    RFRT_MUTE_GRP      = "_PcmdMuteGrp",
    RFRT_LEVEL         = "_PcmdLevel",
    RFRT_SRC_KEY       = "_PcmdSrcKey",
    RFRT_PIT_FINE      = "_PcmdPitFine",
    RFRT_PIT_RND       = "_PcmdPitRnd",
    RFRT_PAN           = "_PcmdPan",
    RFRT_PAN_RND       = "_PcmdPanRnd",
    RFRT_PAN_ALT       = "_PcmdPanAlt",
    RFRT_ENV_MODE      = "_PcmdEnvMode",
    RFRT_DRY_SEND      = "_PcmdDrySend",
    RFRT_CHO_SEND_A    = "_PcmdChoSend",
    RFRT_REV_SEND_A    = "_PcmdRevSend",
    RFRT_OUT_ASGN      = "_PcmdOutAsgn",
    RFRT_BEND_RANGE    = "_PcmdBendRange",
    RFRT_RX_EXPR       = "_PcmdRxExpr",
    RFRT_RX_HOLD       = "_PcmdRxHold",
    RFRT_ONE_SHOT      = "_PcmdOneShot",
    RFRT_LEVEL_MOD     = "_PcmdLevelMod",
    RFRT_WMT_VELO_CTRL = "_PcmdWmtVeloCtrl",
    -- WMT1
    RFRT_WMT1_SW         = "_PcmdWmt1Sw",
    RFRT_WMT1_WAV_GTYPE  = "_PcmdWmt1WavGtype",
    RFRT_WMT1_WAV_GID    = "_PcmdWmt1WavGid",
    RFRT_WMT1_WAV_NUML   = "_PcmdWmt1WavNumL",
    RFRT_WMT1_WAV_NUMR   = "_PcmdWmt1WavNumR",
    RFRT_WMT1_WAV_GAIN   = "_PcmdWmt1WavGain",
    RFRT_WMT1_FXM_SW     = "_PcmdWmt1FxmSw",
    RFRT_WMT1_FXM_COLOR  = "_PcmdWmt1FxmColor",
    RFRT_WMT1_FXM_DEPTH  = "_PcmdWmt1FxmDepth",
    RFRT_WMT1_TEMPO_SYNC = "_PcmdWmt1TempoSync",
    RFRT_WMT1_PIT_CRS    = "_PcmdWmt1PitCrs",
    RFRT_WMT1_PIT_FINE   = "_PcmdWmt1PitFine",
    RFRT_WMT1_PAN        = "_PcmdWmt1Pan",
    RFRT_WMT1_PAN_RND    = "_PcmdWmt1PanRnd",
    RFRT_WMT1_PAN_ALT    = "_PcmdWmt1PanAlt",
    RFRT_WMT1_LEVEL      = "_PcmdWmt1Level",
    RFRT_WMT1_VRANGE_LO  = "_PcmdWmt1VrangeLo",
    RFRT_WMT1_VRANGE_UP  = "_PcmdWmt1VrangeUp",
    RFRT_WMT1_VFADE_LO   = "_PcmdWmt1VfadeLo",
    RFRT_WMT1_VFADE_UP   = "_PcmdWmt1VfadeUp",
    -- WMT2
    RFRT_WMT2_SW         = "_PcmdWmt2Sw",
    RFRT_WMT2_WAV_GTYPE  = "_PcmdWmt2WavGtype",
    RFRT_WMT2_WAV_GID    = "_PcmdWmt2WavGid",
    RFRT_WMT2_WAV_NUML   = "_PcmdWmt2WavNumL",
    RFRT_WMT2_WAV_NUMR   = "_PcmdWmt2WavNumR",
    RFRT_WMT2_WAV_GAIN   = "_PcmdWmt2WavGain",
    RFRT_WMT2_FXM_SW     = "_PcmdWmt2FxmSw",
    RFRT_WMT2_FXM_COLOR  = "_PcmdWmt2FxmColor",
    RFRT_WMT2_FXM_DEPTH  = "_PcmdWmt2FxmDepth",
    RFRT_WMT2_TEMPO_SYNC = "_PcmdWmt2TempoSync",
    RFRT_WMT2_PIT_CRS    = "_PcmdWmt2PitCrs",
    RFRT_WMT2_PIT_FINE   = "_PcmdWmt2PitFine",
    RFRT_WMT2_PAN        = "_PcmdWmt2Pan",
    RFRT_WMT2_PAN_RND    = "_PcmdWmt2PanRnd",
    RFRT_WMT2_PAN_ALT    = "_PcmdWmt2PanAlt",
    RFRT_WMT2_LEVEL      = "_PcmdWmt2Level",
    RFRT_WMT2_VRANGE_LO  = "_PcmdWmt2VrangeLo",
    RFRT_WMT2_VRANGE_UP  = "_PcmdWmt2VrangeUp",
    RFRT_WMT2_VFADE_LO   = "_PcmdWmt2VfadeLo",
    RFRT_WMT2_VFADE_UP   = "_PcmdWmt2VfadeUp",
    -- WMT3
    RFRT_WMT3_SW         = "_PcmdWmt3Sw",
    RFRT_WMT3_WAV_GTYPE  = "_PcmdWmt3WavGtype",
    RFRT_WMT3_WAV_GID    = "_PcmdWmt3WavGid",
    RFRT_WMT3_WAV_NUML   = "_PcmdWmt3WavNumL",
    RFRT_WMT3_WAV_NUMR   = "_PcmdWmt3WavNumR",
    RFRT_WMT3_WAV_GAIN   = "_PcmdWmt3WavGain",
    RFRT_WMT3_FXM_SW     = "_PcmdWmt3FxmSw",
    RFRT_WMT3_FXM_COLOR  = "_PcmdWmt3FxmColor",
    RFRT_WMT3_FXM_DEPTH  = "_PcmdWmt3FxmDepth",
    RFRT_WMT3_TEMPO_SYNC = "_PcmdWmt3TempoSync",
    RFRT_WMT3_PIT_CRS    = "_PcmdWmt3PitCrs",
    RFRT_WMT3_PIT_FINE   = "_PcmdWmt3PitFine",
    RFRT_WMT3_PAN        = "_PcmdWmt3Pan",
    RFRT_WMT3_PAN_RND    = "_PcmdWmt3PanRnd",
    RFRT_WMT3_PAN_ALT    = "_PcmdWmt3PanAlt",
    RFRT_WMT3_LEVEL      = "_PcmdWmt3Level",
    RFRT_WMT3_VRANGE_LO  = "_PcmdWmt3VrangeLo",
    RFRT_WMT3_VRANGE_UP  = "_PcmdWmt3VrangeUp",
    RFRT_WMT3_VFADE_LO   = "_PcmdWmt3VfadeLo",
    RFRT_WMT3_VFADE_UP   = "_PcmdWmt3VfadeUp",
    -- WMT4
    RFRT_WMT4_SW         = "_PcmdWmt4Sw",
    RFRT_WMT4_WAV_GTYPE  = "_PcmdWmt4WavGtype",
    RFRT_WMT4_WAV_GID    = "_PcmdWmt4WavGid",
    RFRT_WMT4_WAV_NUML   = "_PcmdWmt4WavNumL",
    RFRT_WMT4_WAV_NUMR   = "_PcmdWmt4WavNumR",
    RFRT_WMT4_WAV_GAIN   = "_PcmdWmt4WavGain",
    RFRT_WMT4_FXM_SW     = "_PcmdWmt4FxmSw",
    RFRT_WMT4_FXM_COLOR  = "_PcmdWmt4FxmColor",
    RFRT_WMT4_FXM_DEPTH  = "_PcmdWmt4FxmDepth",
    RFRT_WMT4_TEMPO_SYNC = "_PcmdWmt4TempoSync",
    RFRT_WMT4_PIT_CRS    = "_PcmdWmt4PitCrs",
    RFRT_WMT4_PIT_FINE   = "_PcmdWmt4PitFine",
    RFRT_WMT4_PAN        = "_PcmdWmt4Pan",
    RFRT_WMT4_PAN_RND    = "_PcmdWmt4PanRnd",
    RFRT_WMT4_PAN_ALT    = "_PcmdWmt4PanAlt",
    RFRT_WMT4_LEVEL      = "_PcmdWmt4Level",
    RFRT_WMT4_VRANGE_LO  = "_PcmdWmt4VrangeLo",
    RFRT_WMT4_VRANGE_UP  = "_PcmdWmt4VrangeUp",
    RFRT_WMT4_VFADE_LO   = "_PcmdWmt4VfadeLo",
    RFRT_WMT4_VFADE_UP   = "_PcmdWmt4VfadeUp",
    -- Pitch Env
    RFRT_PENV_DEPTH     = "_PcmdPenvDepth",
    RFRT_PENV_VSENS     = "_PcmdPenvVsens",
    RFRT_PENV_T1_VSENS  = "_PcmdPenvT1Vsens",
    RFRT_PENV_T4_VSENS  = "_PcmdPenvT4Vsens",
    RFRT_PENV_T1        = "_PcmdPenvT1",
    RFRT_PENV_T2        = "_PcmdPenvT2",
    RFRT_PENV_T3        = "_PcmdPenvT3",
    RFRT_PENV_T4        = "_PcmdPenvT4",
    RFRT_PENV_L0        = "_PcmdPenvL0",
    RFRT_PENV_L1        = "_PcmdPenvL1",
    RFRT_PENV_L2        = "_PcmdPenvL2",
    RFRT_PENV_L3        = "_PcmdPenvL3",
    RFRT_PENV_L4        = "_PcmdPenvL4",
    -- TVF
    RFRT_FILTER_TYPE    = "_PcmdFilterType",
    RFRT_CUTOFF         = "_PcmdCutoff",
    RFRT_CUTOFF_VCRV    = "_PcmdCutoffVcrv",
    RFRT_CUTOFF_VSENS   = "_PcmdCutoffVsens",
    RFRT_RESO           = "_PcmdReso",
    RFRT_RESO_VSENS     = "_PcmdResoVsens",
    RFRT_FENV_DEPTH     = "_PcmdFenvDepth",
    RFRT_FENV_VCRV      = "_PcmdFenvVcrv",
    RFRT_FENV_VSENS     = "_PcmdFenvVsens",
    RFRT_FENV_T1_VSENS  = "_PcmdFenvT1Vsens",
    RFRT_FENV_T4_VSENS  = "_PcmdFenvT4Vsens",
    RFRT_FENV_T1        = "_PcmdFenvT1",
    RFRT_FENV_T2        = "_PcmdFenvT2",
    RFRT_FENV_T3        = "_PcmdFenvT3",
    RFRT_FENV_T4        = "_PcmdFenvT4",
    RFRT_FENV_L0        = "_PcmdFenvL0",
    RFRT_FENV_L1        = "_PcmdFenvL1",
    RFRT_FENV_L2        = "_PcmdFenvL2",
    RFRT_FENV_L3        = "_PcmdFenvL3",
    RFRT_FENV_L4        = "_PcmdFenvL4",
    -- TVA
    RFRT_LEVEL_VCRV     = "_PcmdLevelVcrv",
    RFRT_LEVEL_VSENS    = "_PcmdLevelVsens",
    RFRT_AENV_T1_VSENS  = "_PcmdAenvT1Vsens",
    RFRT_AENV_T4_VSENS  = "_PcmdAenvT4Vsens",
    RFRT_AENV_T1        = "_PcmdAenvT1",
    RFRT_AENV_T2        = "_PcmdAenvT2",
    RFRT_AENV_T3        = "_PcmdAenvT3",
    RFRT_AENV_T4        = "_PcmdAenvT4",
    RFRT_AENV_L1        = "_PcmdAenvL1",
    RFRT_AENV_L2        = "_PcmdAenvL2",
    RFRT_AENV_L3        = "_PcmdAenvL3",
}

-- Per-part: currently selected note (21-88); default = 36 (C2)
-- Shared across Pitch, WMT 1-4 sections in the same part
local notePartMap = {36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36}

-- Creates all per-pitch sections: Pitch (Common + Env + TVF + TVA) and WMT 1-4
-- All sections in the same part share notePartMap, so the note selector in any
-- section updates routing for all other sections in that part.
function CreatePcmdPitchSections(main)
    for partNr = 1, 16, 1 do
        local pn = string.format("%02d", partNr)

        -- One receive handler per part covers ALL RFRT_* params (Pitch + WMT1-4)
        local function pitchInstHandler(leafNode, response)
            if not IsIdForPart(leafNode.fullid, partNr) then return nil end
            local currentNote = notePartMap[partNr]
            if not string.find(leafNode.fullid, "_RT" .. currentNote .. "-", 1, true) then
                return nil
            end
            local suffix = nodeToFakeSuffix[leafNode.node.id]
            if suffix == nil then return nil end
            local msg = ValueChangedMessage.new()
            msg.id = "PRM-_FPART" .. partNr .. suffix
            msg.i7Value = Bytes_To_Value(response.payload)
            return msg
        end
        AddReceiveHandler(pitchInstHandler)

        -- Routes a setValue to the current note's RFRT_ address
        local function makePitchSetter(rfrtName)
            return function(i7value)
                local note = notePartMap[partNr]
                local realId = "PRM-_FPART" .. partNr .. "-_RHY-_RT" .. note .. "-RFRT_" .. rfrtName
                return CreateSysexMessage(realId, i7value)
            end
        end

        local function pid(suffix)
            return "PRM-_FPART" .. partNr .. suffix
        end

        -- Note selector param — each section gets a unique ID to avoid shared-ptr
        -- overwrites in the C++ parameterDefs map. All still write to notePartMap[partNr].
        local function noteSelector(id)
            return {
                type="select", id=id, name=get("Pitch (Note)"),
                default=15,  -- index 15 = note 36 (C2)
                options=NoteOptions,
                setValue=function(i7value)
                    notePartMap[partNr] = 21 + math.tointeger(i7value)
                    return {}
                end
            }
        end

        local function rxSysex()
            return function()
                local note = notePartMap[partNr]
                return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_RHY-_RT" .. note)
            end
        end

        -- ----------------------------------------------------------------
        -- Pitch section: Partial Common + Pitch Env + TVF + TVA
        -- ----------------------------------------------------------------
        local kPitch = "Part " .. pn .. " PCM-D Pitch"
        local pitchParams = {
            noteSelector(pid("_PcmdNote")),

            -- Partial Common
            {type="select", id=pid("_PcmdAsgnType"),  name=get("Assign Type"),         default=0,   options=AssignType,            setValue=makePitchSetter("ASGN_TYPE")},
            {type="range",  id=pid("_PcmdMuteGrp"),   name=get("Mute Group"),          default=0,   min=get(0),   max=get(31),   setValue=makePitchSetter("MUTE_GRP")},
            {type="range",  id=pid("_PcmdLevel"),      name=get("Level"),               default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("LEVEL")},
            {type="range",  id=pid("_PcmdSrcKey"),     name=get("Source Key"),          default=60,  min=get(0),   max=get(127),  setValue=makePitchSetter("SRC_KEY")},
            {type="range",  id=pid("_PcmdPitFine"),    name=get("Fine Tune"),           default=0,   min=get(-50), max=get(50),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PIT_FINE")},
            {type="range",  id=pid("_PcmdPitRnd"),     name=get("Random Pitch Depth"),  default=0,   min=get(0),   max=get(30),   setValue=makePitchSetter("PIT_RND")},
            {type="range",  id=pid("_PcmdPan"),        name=get("Pan"),                 default=0,   min=get(-64), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PAN")},
            {type="range",  id=pid("_PcmdPanRnd"),     name=get("Random Pan Depth"),    default=0,   min=get(0),   max=get(63),   setValue=makePitchSetter("PAN_RND")},
            {type="range",  id=pid("_PcmdPanAlt"),     name=get("Alt Pan Depth"),       default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PAN_ALT")},
            {type="select", id=pid("_PcmdEnvMode"),    name=get("Env Mode"),            default=1,   options=EnvMode,               setValue=makePitchSetter("ENV_MODE")},
            {type="range",  id=pid("_PcmdDrySend"),    name=get("Dry Send Level"),      default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("DRY_SEND")},
            {type="range",  id=pid("_PcmdChoSend"),    name=get("Chorus Send Level"),   default=0,   min=get(0),   max=get(127),  setValue=makePitchSetter("CHO_SEND_A")},
            {type="range",  id=pid("_PcmdRevSend"),    name=get("Reverb Send Level"),   default=64,  min=get(0),   max=get(127),  setValue=makePitchSetter("REV_SEND_A")},
            {type="select", id=pid("_PcmdOutAsgn"),    name=get("Output Assign"),       default=0,   options=OutAssign,             setValue=makePitchSetter("OUT_ASGN")},
            {type="range",  id=pid("_PcmdBendRange"),  name=get("Pitch Bend Range"),    default=2,   min=get(0),   max=get(48),   setValue=makePitchSetter("BEND_RANGE")},
            {type="toggle", id=pid("_PcmdRxExpr"),     name=get("Rx Expression"),       default=1,   min=get(0),   max=get(1),    setValue=makePitchSetter("RX_EXPR")},
            {type="toggle", id=pid("_PcmdRxHold"),     name=get("Rx Hold-1"),           default=1,   min=get(0),   max=get(1),    setValue=makePitchSetter("RX_HOLD")},
            {type="toggle", id=pid("_PcmdOneShot"),    name=get("One Shot Mode"),       default=0,   min=get(0),   max=get(1),    setValue=makePitchSetter("ONE_SHOT")},
            {type="range",  id=pid("_PcmdLevelMod"),   name=get("Relative Level"),      default=0,   min=get(-64), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("LEVEL_MOD")},
            {type="select", id=pid("_PcmdWmtVeloCtrl"), name=get("WMT Velo Ctrl"),      default=1,   options=WmtVeloCtrl,           setValue=makePitchSetter("WMT_VELO_CTRL")},

            -- Pitch Envelope
            {type="range", id=pid("_PcmdPenvDepth"),    name=get("Pitch Env Depth"),    default=0,   min=get(-12), max=get(12),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_DEPTH")},
            {type="range", id=pid("_PcmdPenvVsens"),    name=get("Pitch Env V-Sens"),   default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_VSENS")},
            {type="range", id=pid("_PcmdPenvT1Vsens"),  name=get("Pitch Env T1 V-Sns"), default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_T1_VSENS")},
            {type="range", id=pid("_PcmdPenvT4Vsens"),  name=get("Pitch Env T4 V-Sns"), default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_T4_VSENS")},
            {type="range", id=pid("_PcmdPenvT1"),       name=get("Pitch Env T1"),       default=0,   min=get(0),   max=get(127), setValue=makePitchSetter("PENV_T1")},
            {type="range", id=pid("_PcmdPenvL1"),       name=get("Pitch Env L1"),       default=-30, min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_L1")},
            {type="range", id=pid("_PcmdPenvT2"),       name=get("Pitch Env T2"),       default=40,  min=get(0),   max=get(127), setValue=makePitchSetter("PENV_T2")},
            {type="range", id=pid("_PcmdPenvL2"),       name=get("Pitch Env L2"),       default=30,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_L2")},
            {type="range", id=pid("_PcmdPenvT3"),       name=get("Pitch Env T3"),       default=80,  min=get(0),   max=get(127), setValue=makePitchSetter("PENV_T3")},
            {type="range", id=pid("_PcmdPenvL3"),       name=get("Pitch Env L3"),       default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_L3")},
            {type="range", id=pid("_PcmdPenvT4"),       name=get("Pitch Env T4"),       default=40,  min=get(0),   max=get(127), setValue=makePitchSetter("PENV_T4")},
            {type="range", id=pid("_PcmdPenvL0"),       name=get("Pitch Env L0"),       default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_L0")},
            {type="range", id=pid("_PcmdPenvL4"),       name=get("Pitch Env L4"),       default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("PENV_L4")},

            -- TVF
            {type="select", id=pid("_PcmdFilterType"),   name=get("Filter Type"),       default=1,   options=FilterType,            setValue=makePitchSetter("FILTER_TYPE")},
            {type="range",  id=pid("_PcmdCutoff"),       name=get("Cutoff Freq"),       default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("CUTOFF")},
            {type="range",  id=pid("_PcmdReso"),         name=get("Resonance"),         default=0,   min=get(0),   max=get(127),  setValue=makePitchSetter("RESO")},
            {type="select", id=pid("_PcmdCutoffVcrv"),   name=get("Cutoff V-Curve"),    default=1,   options=VCurve,                setValue=makePitchSetter("CUTOFF_VCRV")},
            {type="range",  id=pid("_PcmdCutoffVsens"),  name=get("Cutoff V-Sens"),     default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("CUTOFF_VSENS")},
            {type="range",  id=pid("_PcmdResoVsens"),    name=get("Resonance V-Sens"),  default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("RESO_VSENS")},
            {type="range",  id=pid("_PcmdFenvDepth"),    name=get("TVF Env Depth"),     default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("FENV_DEPTH")},
            {type="select", id=pid("_PcmdFenvVcrv"),     name=get("TVF Env V-Curve"),   default=1,   options=VCurve,                setValue=makePitchSetter("FENV_VCRV")},
            {type="range",  id=pid("_PcmdFenvVsens"),    name=get("TVF Env V-Sens"),    default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("FENV_VSENS")},
            {type="range",  id=pid("_PcmdFenvT1Vsens"),  name=get("TVF Env T1 V-Sns"),  default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("FENV_T1_VSENS")},
            {type="range",  id=pid("_PcmdFenvT4Vsens"),  name=get("TVF Env T4 V-Sns"),  default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("FENV_T4_VSENS")},
            {type="range",  id=pid("_PcmdFenvT1"),       name=get("TVF Env T1"),        default=0,   min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_T1")},
            {type="range",  id=pid("_PcmdFenvL1"),       name=get("TVF Env L1"),        default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_L1")},
            {type="range",  id=pid("_PcmdFenvT2"),       name=get("TVF Env T2"),        default=10,  min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_T2")},
            {type="range",  id=pid("_PcmdFenvL2"),       name=get("TVF Env L2"),        default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_L2")},
            {type="range",  id=pid("_PcmdFenvT3"),       name=get("TVF Env T3"),        default=10,  min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_T3")},
            {type="range",  id=pid("_PcmdFenvL3"),       name=get("TVF Env L3"),        default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_L3")},
            {type="range",  id=pid("_PcmdFenvT4"),       name=get("TVF Env T4"),        default=64,  min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_T4")},
            {type="range",  id=pid("_PcmdFenvL0"),       name=get("TVF Env L0"),        default=0,   min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_L0")},
            {type="range",  id=pid("_PcmdFenvL4"),       name=get("TVF Env L4"),        default=0,   min=get(0),   max=get(127),  setValue=makePitchSetter("FENV_L4")},

            -- TVA + TVA Envelope
            {type="select", id=pid("_PcmdLevelVcrv"),    name=get("TVA Level V-Curve"), default=1,   options=VCurve,                setValue=makePitchSetter("LEVEL_VCRV")},
            {type="range",  id=pid("_PcmdLevelVsens"),   name=get("TVA Level V-Sens"),  default=32,  min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("LEVEL_VSENS")},
            {type="range",  id=pid("_PcmdAenvT1Vsens"),  name=get("TVA Env T1 V-Sns"),  default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("AENV_T1_VSENS")},
            {type="range",  id=pid("_PcmdAenvT4Vsens"),  name=get("TVA Env T4 V-Sns"),  default=0,   min=get(-63), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("AENV_T4_VSENS")},
            {type="range",  id=pid("_PcmdAenvT1"),       name=get("TVA Env T1"),        default=0,   min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_T1")},
            {type="range",  id=pid("_PcmdAenvL1"),       name=get("TVA Env L1"),        default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_L1")},
            {type="range",  id=pid("_PcmdAenvT2"),       name=get("TVA Env T2"),        default=10,  min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_T2")},
            {type="range",  id=pid("_PcmdAenvL2"),       name=get("TVA Env L2"),        default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_L2")},
            {type="range",  id=pid("_PcmdAenvT3"),       name=get("TVA Env T3"),        default=10,  min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_T3")},
            {type="range",  id=pid("_PcmdAenvL3"),       name=get("TVA Env L3"),        default=127, min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_L3")},
            {type="range",  id=pid("_PcmdAenvT4"),       name=get("TVA Env T4"),        default=10,  min=get(0),   max=get(127),  setValue=makePitchSetter("AENV_T4")},
        }
        main[kPitch] = {name=kPitch, params=pitchParams, getReceiveValueSysex=rxSysex()}

        -- ----------------------------------------------------------------
        -- WMT Note section (note selector rendered above WMT tab bar)
        -- ----------------------------------------------------------------
        local kWmtNote = "Part " .. pn .. " PCM-D WMT Note"
        main[kWmtNote] = {
            name   = kWmtNote,
            params = { noteSelector(pid("_PcmdNoteWmt")) },
        }

        -- ----------------------------------------------------------------
        -- WMT sections 1-4 (tabbed in _pcmdTabsSection)
        -- ----------------------------------------------------------------
        currentGidPcmd[partNr] = currentGidPcmd[partNr] or {}
        for wmtNr = 1, 4, 1 do
            local s      = tostring(wmtNr)
            local kW     = "Part " .. pn .. " PCM-D WMT " .. s
            local wmtNr2 = wmtNr  -- capture for closures

            local origGtypeSetter = makePitchSetter("WMT"..s.."_WAV_GTYPE")
            local wmtParams = {
                {type="toggle", id=pid("_PcmdWmt"..s.."Sw"),       name=get("Wave Switch"),        default=0,   min=get(0),   max=get(1),   setValue=makePitchSetter("WMT"..s.."_SW")},
                {type="select", id=pid("_PcmdWmt"..s.."WavGtype"),  name=get("Wave Group"),         default=0,   options=WaveGroup,
                    setValue=function(v)
                        currentGidPcmd[partNr][wmtNr2] = v
                        return origGtypeSetter(v)
                    end},
                {type="select", id=pid("_PcmdWmt"..s.."WavNumL"),   name=get("Wave No. L (Mono)"),  default=0,
                    options=function() return waveOptionsForPcmd(partNr, wmtNr2) end,
                    setValue=makePitchSetter("WMT"..s.."_WAV_NUML")},
                {type="select", id=pid("_PcmdWmt"..s.."WavNumR"),   name=get("Wave No. R"),         default=0,
                    options=function() return waveOptionsForPcmd(partNr, wmtNr2) end,
                    setValue=makePitchSetter("WMT"..s.."_WAV_NUMR")},
                {type="select", id=pid("_PcmdWmt"..s.."WavGain"),   name=get("Wave Gain"),          default=1,   options=WaveGain,             setValue=makePitchSetter("WMT"..s.."_WAV_GAIN")},
                {type="toggle", id=pid("_PcmdWmt"..s.."FxmSw"),     name=get("FXM Switch"),        default=0,   min=get(0),   max=get(1),   setValue=makePitchSetter("WMT"..s.."_FXM_SW")},
                {type="select", id=pid("_PcmdWmt"..s.."FxmColor"),  name=get("FXM Color"),         default=0,   options=FxmColor,             setValue=makePitchSetter("WMT"..s.."_FXM_COLOR")},
                {type="range",  id=pid("_PcmdWmt"..s.."FxmDepth"),  name=get("FXM Depth"),         default=0,   min=get(0),   max=get(16),  setValue=makePitchSetter("WMT"..s.."_FXM_DEPTH")},
                {type="toggle", id=pid("_PcmdWmt"..s.."TempoSync"), name=get("Tempo Sync"),        default=0,   min=get(0),   max=get(1),   setValue=makePitchSetter("WMT"..s.."_TEMPO_SYNC")},
                {type="range",  id=pid("_PcmdWmt"..s.."PitCrs"),    name=get("Coarse Tune"),       default=0,   min=get(-48), max=get(48),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("WMT"..s.."_PIT_CRS")},
                {type="range",  id=pid("_PcmdWmt"..s.."PitFine"),   name=get("Fine Tune"),         default=0,   min=get(-50), max=get(50),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("WMT"..s.."_PIT_FINE")},
                {type="range",  id=pid("_PcmdWmt"..s.."Pan"),       name=get("Pan"),               default=0,   min=get(-64), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makePitchSetter("WMT"..s.."_PAN")},
                {type="toggle", id=pid("_PcmdWmt"..s.."PanRnd"),    name=get("Random Pan"),        default=1,   min=get(0),   max=get(1),   setValue=makePitchSetter("WMT"..s.."_PAN_RND")},
                {type="select", id=pid("_PcmdWmt"..s.."PanAlt"),    name=get("Alt Pan"),           default=1,   options=AltPanSw,             setValue=makePitchSetter("WMT"..s.."_PAN_ALT")},
                {type="range",  id=pid("_PcmdWmt"..s.."Level"),     name=get("Level"),             default=127, min=get(0),   max=get(127), setValue=makePitchSetter("WMT"..s.."_LEVEL")},
                {type="range",  id=pid("_PcmdWmt"..s.."VrangeLo"),  name=get("Velocity Lo"),       default=1,   min=get(1),   max=get(127), setValue=makePitchSetter("WMT"..s.."_VRANGE_LO")},
                {type="range",  id=pid("_PcmdWmt"..s.."VrangeUp"),  name=get("Velocity Up"),       default=127, min=get(1),   max=get(127), setValue=makePitchSetter("WMT"..s.."_VRANGE_UP")},
                {type="range",  id=pid("_PcmdWmt"..s.."VfadeLo"),   name=get("Velocity Fade Lo"),  default=0,   min=get(0),   max=get(127), setValue=makePitchSetter("WMT"..s.."_VFADE_LO")},
                {type="range",  id=pid("_PcmdWmt"..s.."VfadeUp"),   name=get("Velocity Fade Up"),  default=0,   min=get(0),   max=get(127), setValue=makePitchSetter("WMT"..s.."_VFADE_UP")},
            }
            main[kW] = {name=kW, params=wmtParams, getReceiveValueSysex=rxSysex()}
        end
    end
end
