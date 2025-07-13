require "math"
require "_snaData"

local function i7offsetValue(offset)
    return function (guiValue)
        return math.tointeger(offset + guiValue)
    end
end

local function idTmpl(snaId)
    return "PRM-_FPARTxxx-_SNTONE-_SNTC-SNTC_" .. snaId
end


local snaTemplate = {
    name = "SN-A",
    sub =
    {
        {
            name = "Common",
            params =
            {
              {type="select", id=idTmpl("CATE"), name="Tone Category", min=0, max=46, default=0, options = ToneCategories},
              {type="range", id=idTmpl("PHRASE_OCT"), name="Phrase Oct Shift", min=-3, max=3, default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("TONE_LEVEL"), name="Tone Level", min=0, max=127, default=127, format="%.0f"},
              {type="select", id=idTmpl("MONO_POLY"), name="Mono/Poly", min=0, max=1, default=1, options = MonoPoly},
              {type="range", id=idTmpl("OCTAVE"), name="Oct Shift", min=-3, max=3, default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("PORT_TIME"), name="Portamento Time Offset", min=-64, max=63, default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("VIB_RATE"), name="Vibrato Rate", min=-64, max=63, default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("VIB_DEPTH"), name="Vibrato Depth", min=-64, max=63, default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("VIB_DELAY"), name="Vibrato Delay", min=-64, max=63, default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
            }
        }
    }
}

function CreateSnaSections(main)
    for i = 1, 16, 1 do
        local k = "SNA_" .. string.format("%02d", i)
        local name = k
        local snaData = DeepCopy(snaTemplate);
        snaData.name = name
        if i==1 then
            snaData.isOpen = true
        end
        for key, subSection in ipairs(snaData.sub) do
            for key, param in ipairs(subSection.params) do
                param.id = CreateId(param.id, i)
            end
        end
        main[k] = snaData
    end
end
