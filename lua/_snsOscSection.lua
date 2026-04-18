require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local OscWave    = {[0]="SAW", [1]="SQR", [2]="PW-SQR", [3]="TRI", [4]="SINE", [5]="NOISE", [6]="SUPER-SAW", [7]="PCM"}
local OscWaveVar = {[0]="A", [1]="B", [2]="C"}
local WaveGain   = {[0]="-6dB", [1]="0dB", [2]="+6dB", [3]="+12dB"}

function CreateSnsOscSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 3, 1 do
            local k = "Part " .. string.format("%02d", partNr) .. " SN-S OSC Partial " .. partialNr

            local function p(name)
                return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr .. "-SHPT_" .. name
            end

            local params = {
                {type="select", id=p("OSC_WAVE"),     name=get("OSC Wave"),         default=0,   options=OscWave},
                {type="select", id=p("OSC_WAVE_VAR"), name=get("Wave Variation"),   default=0,   options=OscWaveVar},
                {type="range",  id=p("OSC_PWM"),      name=get("Pulse Width Mod"),  default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=p("OSC_PW"),       name=get("Pulse Width"),      default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=p("OSC_PW_SHIFT"), name=get("PW Shift"),         default=127, min=get(0),   max=get(127), format="%.0f"},
                {type="select", id=p("WAV_GAIN"),     name=get("Wave Gain"),        default=1,   options=WaveGain},
                {type="range",  id=p("WAV_NUML"),     name=get("Wave Number"),      default=45,  min=get(1),   max=get(450), format="%.0f"},
                {type="range",  id=p("HPF_CUTOFF"),   name=get("HPF Cutoff"),       default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=p("SSAW_DETUNE"),  name=get("Super Saw Detune"), default=0,   min=get(0),   max=get(127), format="%.0f"},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = "Part " .. string.format("%02d", partNr) .. " SN-S OSC Partial " .. partialNr,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr)
                end,
            }
        end
    end
end
