package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_model"
require "_com"
require "_snaSection"
require "_sysex"
require "_patchesSection"

local p = ParameterSetValueWrapper;
local get = GetWrapper

Main = {
    parts = {
        name = "Parts View",
        isOpen = true,
        params = {
            p({type="range", id="PRM-_PRF-_FP1-NEFP_LEVEL", name=get("Part 1 Level"), min=get(0), max=get(127)}),
        }
    },
}

CreateSnaSections(Main)
CreatePatchesSections(Main)