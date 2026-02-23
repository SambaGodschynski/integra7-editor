package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_model"
require "_com"
require "_snaSection"
require "_sysex"
require "_patchesSection"
require "_partsSection"
require "_mfxSection"
require "_pcmsCommonSection"
require "_pcmsWaveSection"
require "_pcmsPmtSection"
require "_pcmsPitchSection"
require "_pcmsPitchEnvSection"
require "_pcmsTvfSection"
require "_pcmsTvfEnvSection"
require "_pcmsTvaSection"
require "_pcmsTvaEnvSection"
require "_pcmsOutputSection"
require "_pcmsLfoSection"
require "_pcmsCtrlSection"
require "_pcmsMtrxCtrlSection"

Main = {}

CreateSnaSections(Main)
CreatePatchesSections(Main)
CreatePartsSections(Main)
CreateMfxSections(Main)
CreatePcmsCommonSections(Main)
CreatePcmsWaveSections(Main)
CreatePcmsPmtSections(Main)
CreatePcmsPitchSections(Main)
CreatePcmsPitchEnvSections(Main)
CreatePcmsTvfSections(Main)
CreatePcmsTvfEnvSections(Main)
CreatePcmsTvaSections(Main)
CreatePcmsTvaEnvSections(Main)
CreatePcmsOutputSections(Main)
CreatePcmsLfoSections(Main)
CreatePcmsCtrlSections(Main)
CreatePcmsMtrxCtrlSections(Main)