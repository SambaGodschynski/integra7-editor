package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_model"
require "_com"
require "_snaSection"
require "_sysex"
require "_patchesSection"
require "_partsSection"
require "_mfxSection"

Main = {}

CreateSnaSections(Main)
CreatePatchesSections(Main)
CreatePartsSections(Main)
CreateMfxSections(Main)